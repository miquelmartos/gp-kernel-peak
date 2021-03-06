/* Copyright (c) 2012 Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/iommu.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>

#include <mach/iommu_hw-v2.h>
#include <mach/iommu.h>

static void msm_iommu_reset(void __iomem *base)
{
	int i;

	SET_ACR(base, 0);
	SET_NSACR(base, 0);
	SET_CR2(base, 0);
	SET_NSCR2(base, 0);
	SET_GFAR(base, 0);
	SET_GFSRRESTORE(base, 0);
	SET_TLBIALLNSNH(base, 0);
	SET_PMCR(base, 0);
	SET_SCR1(base, 0);
	SET_SSDR_N(base, 0, 0);

	for (i = 0; i < MAX_NUM_SMR; i++)
		SET_SMR_VALID(base, i, 0);

	mb();
}

static int msm_iommu_parse_dt(struct platform_device *pdev,
				struct msm_iommu_drvdata *drvdata)
{
	struct device_node *child;
	int ret;

	ret = device_move(&pdev->dev, &msm_iommu_root_dev->dev, DPM_ORDER_NONE);
	if (ret)
		return ret;

	for_each_child_of_node(pdev->dev.of_node, child) {
		drvdata->ncb++;
		if (!of_platform_device_create(child, NULL, &pdev->dev))
			pr_err("Failed to create %s device\n", child->name);
	}

	drvdata->name = dev_name(&pdev->dev);
	return 0;
}

static atomic_t msm_iommu_next_id = ATOMIC_INIT(-1);

static int __devinit msm_iommu_probe(struct platform_device *pdev)
{
	struct msm_iommu_drvdata *drvdata;
	struct resource *r;
	int ret;

	if (msm_iommu_root_dev == pdev)
		return 0;

	if (pdev->id == -1)
		pdev->id = atomic_inc_return(&msm_iommu_next_id) - 1;

	drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -EINVAL;

	drvdata->base = devm_ioremap(&pdev->dev, r->start, resource_size(r));
	if (!drvdata->base)
		return -ENOMEM;

	drvdata->pclk = clk_get(&pdev->dev, "iface_clk");
	if (IS_ERR(drvdata->pclk))
		return PTR_ERR(drvdata->pclk);

	ret = clk_prepare_enable(drvdata->pclk);
	if (ret)
		goto fail_enable;

	drvdata->clk = clk_get(&pdev->dev, "core_clk");
	if (!IS_ERR(drvdata->clk)) {
		if (clk_get_rate(drvdata->clk) == 0) {
			ret = clk_round_rate(drvdata->clk, 1);
			clk_set_rate(drvdata->clk, ret);
		}

		ret = clk_prepare_enable(drvdata->clk);
		if (ret) {
			clk_put(drvdata->clk);
			goto fail_pclk;
		}
	} else
		drvdata->clk = NULL;

	msm_iommu_reset(drvdata->base);

	SET_CR0_SMCFCFG(drvdata->base, 1);
	SET_CR0_USFCFG(drvdata->base, 1);
	SET_CR0_STALLD(drvdata->base, 1);
	SET_CR0_GCFGFIE(drvdata->base, 1);
	SET_CR0_GCFGFRE(drvdata->base, 1);
	SET_CR0_GFIE(drvdata->base, 1);
	SET_CR0_GFRE(drvdata->base, 1);
	SET_CR0_CLIENTPD(drvdata->base, 0);

	ret = msm_iommu_parse_dt(pdev, drvdata);
	if (ret)
		goto fail_clk;

	pr_info("device %s mapped at %p, with %d ctx banks\n",
		drvdata->name, drvdata->base, drvdata->ncb);

	platform_set_drvdata(pdev, drvdata);

	if (drvdata->clk)
		clk_disable_unprepare(drvdata->clk);

	clk_disable_unprepare(drvdata->pclk);

	return 0;

fail_clk:
	if (drvdata->clk) {
		clk_disable_unprepare(drvdata->clk);
		clk_put(drvdata->clk);
	}
fail_pclk:
	clk_disable_unprepare(drvdata->pclk);
fail_enable:
	clk_put(drvdata->pclk);
	return ret;
}

static int __devexit msm_iommu_remove(struct platform_device *pdev)
{
	struct msm_iommu_drvdata *drv = NULL;

	drv = platform_get_drvdata(pdev);
	if (drv) {
		if (drv->clk)
			clk_put(drv->clk);
		clk_put(drv->pclk);
		platform_set_drvdata(pdev, NULL);
	}
	return 0;
}

static int msm_iommu_ctx_parse_dt(struct platform_device *pdev,
				struct msm_iommu_drvdata *drvdata,
				struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	struct resource *r, rp;
	u32 sids[MAX_NUM_SMR];
	int num = 0;
	int irq, i, ret, len = 0;

	irq = platform_get_irq(pdev, 0);
	if (irq > 0) {
		ret = request_threaded_irq(irq, NULL,
				msm_iommu_fault_handler_v2,
				IRQF_ONESHOT | IRQF_SHARED,
				"msm_iommu_nonsecure_irq", pdev);
		if (ret) {
			pr_err("Request IRQ %d failed with ret=%d\n", irq, ret);
			return ret;
		}
	}

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -EINVAL;

	ret = of_address_to_resource(pdev->dev.parent->of_node, 0, &rp);
	if (ret)
		return -EINVAL;

	/* Calculate the context bank number using the base addresses. The
	 * first 8 pages belong to the global address space which is followed
	 * by the context banks, hence subtract by 8 to get the context bank
	 * number.
	 */
	ctx_drvdata->num = ((r->start - rp.start) >> CTX_SHIFT) - 8;

	if (of_property_read_string(pdev->dev.of_node, "qcom,iommu-ctx-name",
					&ctx_drvdata->name))
		ctx_drvdata->name = dev_name(&pdev->dev);

	of_get_property(pdev->dev.of_node, "qcom,iommu-ctx-sids", &len);
	BUG_ON(len >= sizeof(sids));
	if (of_property_read_u32_array(pdev->dev.of_node, "qcom,iommu-ctx-sids",
					sids, len / sizeof(*sids)))
		return -EINVAL;

	/* Program the M2V tables for this context */
	for (i = 0; i < len / sizeof(*sids); i++) {
		for (; num < MAX_NUM_SMR; num++)
			if (GET_SMR_VALID(drvdata->base, num) == 0)
				break;
		BUG_ON(num >= MAX_NUM_SMR);

		SET_SMR_VALID(drvdata->base, num, 1);
		SET_SMR_MASK(drvdata->base, num, 0);
		SET_SMR_ID(drvdata->base, num, sids[i]);

		/* Set VMID = 0 */
		SET_S2CR_N(drvdata->base, num, 0);
		SET_S2CR_CBNDX(drvdata->base, num, ctx_drvdata->num);
		/* Set security bit override to be Non-secure */
		SET_S2CR_NSCFG(drvdata->base, sids[i], 3);

		SET_CBAR_N(drvdata->base, ctx_drvdata->num, 0);
		/* Stage 1 Context with Stage 2 bypass */
		SET_CBAR_TYPE(drvdata->base, ctx_drvdata->num, 1);
		/* Route page faults to the non-secure interrupt */
		SET_CBAR_IRPTNDX(drvdata->base, ctx_drvdata->num, 1);
	}
	mb();

	return 0;
}

static int __devinit msm_iommu_ctx_probe(struct platform_device *pdev)
{
	struct msm_iommu_drvdata *drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata = NULL;
	int ret;

	if (!pdev->dev.parent)
		return -EINVAL;

	drvdata = dev_get_drvdata(pdev->dev.parent);
	if (!drvdata)
		return -ENODEV;

	ctx_drvdata = devm_kzalloc(&pdev->dev, sizeof(*ctx_drvdata),
					GFP_KERNEL);
	if (!ctx_drvdata)
		return -ENOMEM;

	ctx_drvdata->pdev = pdev;
	INIT_LIST_HEAD(&ctx_drvdata->attached_elm);
	platform_set_drvdata(pdev, ctx_drvdata);

	ret = clk_prepare_enable(drvdata->pclk);
	if (ret)
		return ret;

	if (drvdata->clk) {
		ret = clk_prepare_enable(drvdata->clk);
		if (ret) {
			clk_disable_unprepare(drvdata->pclk);
			return ret;
		}
	}

	ret = msm_iommu_ctx_parse_dt(pdev, drvdata, ctx_drvdata);
	if (!ret)
		dev_info(&pdev->dev, "context %s using bank %d\n",
				dev_name(&pdev->dev), ctx_drvdata->num);

	if (drvdata->clk)
		clk_disable_unprepare(drvdata->clk);
	clk_disable_unprepare(drvdata->pclk);

	return ret;
}

static int __devexit msm_iommu_ctx_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct of_device_id msm_iommu_match_table[] = {
	{ .compatible = "qcom,msm-smmu-v2", },
	{}
};

static struct platform_driver msm_iommu_driver = {
	.driver = {
		.name	= "msm_iommu_v2",
		.of_match_table = msm_iommu_match_table,
	},
	.probe		= msm_iommu_probe,
	.remove		= __devexit_p(msm_iommu_remove),
};

static struct of_device_id msm_iommu_ctx_match_table[] = {
	{ .name = "qcom,iommu-ctx", },
	{}
};

static struct platform_driver msm_iommu_ctx_driver = {
	.driver = {
		.name	= "msm_iommu_ctx_v2",
		.of_match_table = msm_iommu_ctx_match_table,
	},
	.probe		= msm_iommu_ctx_probe,
	.remove		= __devexit_p(msm_iommu_ctx_remove),
};

static int __init msm_iommu_driver_init(void)
{
	struct device_node *node;
	int ret;

	node = of_find_compatible_node(NULL, NULL, "qcom,msm-smmu-v2");
	if (!node)
		return -ENODEV;

	of_node_put(node);

	msm_iommu_root_dev = platform_device_register_simple(
						"msm_iommu", -1, 0, 0);
	if (!msm_iommu_root_dev) {
		pr_err("Failed to create root IOMMU device\n");
		ret = -ENODEV;
		goto error;
	}

	atomic_inc(&msm_iommu_next_id);

	ret = platform_driver_register(&msm_iommu_driver);
	if (ret != 0) {
		pr_err("Failed to register IOMMU driver\n");
		goto error;
	}

	ret = platform_driver_register(&msm_iommu_ctx_driver);
	if (ret != 0) {
		pr_err("Failed to register IOMMU context driver\n");
		goto error;
	}

error:
	return ret;
}

static void __exit msm_iommu_driver_exit(void)
{
	platform_driver_unregister(&msm_iommu_ctx_driver);
	platform_driver_unregister(&msm_iommu_driver);
	platform_device_unregister(msm_iommu_root_dev);
}

subsys_initcall(msm_iommu_driver_init);
module_exit(msm_iommu_driver_exit);

MODULE_LICENSE("GPL v2");
