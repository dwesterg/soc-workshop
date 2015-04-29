/*
 * Copyright (C) 2015 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

/* globals */
static int g_demo_driver_base_addr;
static int g_demo_driver_irq;
static unsigned long g_demo_driver_clk_rate;

static int platform_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	int irq;
	struct clk *clk;
	unsigned long clk_rate;

	pr_info("platform_probe enter\n");

	ret_val = -EINVAL;

	/* get our first memory resource */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r != NULL) {
		pr_info("r->start = 0x%08lx\n", (long unsigned int)r->start);
		pr_info("r->end = 0x%08lx\n", (long unsigned int)r->end);
		pr_info("r->name = %s\n", r->name);
		if (r->start & ~PAGE_MASK) {
			pr_err("demo_driver base not aligned to PAGE_SIZE\n");
			goto bad_exit_return;
		}
		if (((r->end - r->start) + 1) > PAGE_SIZE) {
			pr_err("demo_driver span is larger than PAGE_SIZE\n");
			goto bad_exit_return;
		}
	} else {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_return;
	}

	g_demo_driver_base_addr = r->start;

	/* get our interrupt resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("irq not available\n");
		goto bad_exit_return;
	} else {
		pr_info("irq = %d\n", irq);
	}

	g_demo_driver_irq = irq;

	/* get our clock resource */
	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		pr_err("clk not available\n");
		goto bad_exit_return;
	} else {
		clk_rate = clk_get_rate(clk);
		pr_info("clk = %lu HZ\n", clk_rate);
	}

	g_demo_driver_clk_rate = clk_rate;

	pr_info("platform_probe exit\n");
	return 0;

bad_exit_return:
	pr_info("platform_probe bad_exit\n");
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	pr_info("platform_remove enter\n");
	pr_info("platform_remove exit\n");
	return 0;
}

static struct of_device_id demo_driver_dt_ids[] = {
	{
	 .compatible = "demo,driver-1.0"},
	{ /* end of table */ }
};

MODULE_DEVICE_TABLE(of, demo_driver_dt_ids);

static struct platform_driver the_platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		   .name = "demo_driver_4",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
	/*
	   .shutdown = unused,
	   .suspend = unused,
	   .resume = unused,
	   .id_table = unused,
	 */
};

static int demo_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&the_platform_driver);
	if (ret_val != 0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		return ret_val;
	}

	pr_info("demo_init exit\n");
	return 0;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");

	platform_driver_unregister(&the_platform_driver);

	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_DESCRIPTION("Demonstration Module 4 - extract hardware details from DT");
MODULE_VERSION("1.0");
