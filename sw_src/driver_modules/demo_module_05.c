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
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>

#include "demo_module.h"
#include "my_altera_avalon_timer_regs.h"

/* prototypes */
static struct platform_driver the_platform_driver;

/* globals */
static struct semaphore g_dev_probe_sem;
static int g_platform_probe_flag;
static int g_demo_driver_base_addr;
static int g_demo_driver_size;
static int g_demo_driver_irq;
static void __iomem *g_ioremap_addr;
static void __iomem *g_timer_base;

irqreturn_t demo_driver_interrupt_handler(int irq, void *dev_id)
{
	/* clear the interrupt */
	iowrite32(0, IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

	return IRQ_HANDLED;
}

static int platform_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	int irq;
	struct clk *clk;
	unsigned long clk_rate;
	struct resource *demo_driver_mem_region;
	uint32_t io_result;
	uint32_t period_100ms;

	pr_info("platform_probe enter\n");

	ret_val = -EBUSY;

	/* acquire the probe lock */
	if (down_interruptible(&g_dev_probe_sem))
		return -ERESTARTSYS;

	if (g_platform_probe_flag != 0)
		goto bad_exit_return;

	ret_val = -EINVAL;

	/* get our first memory resource */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_return;
	}

	g_demo_driver_base_addr = r->start;
	g_demo_driver_size = resource_size(r);

	/* get our interrupt resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("irq not available\n");
		goto bad_exit_return;
	}

	g_demo_driver_irq = irq;

	/* get our clock resource */
	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		pr_err("clk not available\n");
		goto bad_exit_return;
	} else {
		clk_rate = clk_get_rate(clk);
	}

	ret_val = -EBUSY;

	/* reserve our memory region */
	demo_driver_mem_region = request_mem_region(g_demo_driver_base_addr,
						    g_demo_driver_size,
						    "demo_driver_hw_region");
	if (demo_driver_mem_region == NULL) {
		pr_err("request_mem_region failed: g_demo_driver_base_addr\n");
		goto bad_exit_return;
	}

	/* ioremap our memory region */
	g_ioremap_addr = ioremap(g_demo_driver_base_addr, g_demo_driver_size);
	if (g_ioremap_addr == NULL) {
		pr_err("ioremap failed: g_demo_driver_base_addr\n");
		goto bad_exit_release_mem_region;
	}

	g_timer_base = g_ioremap_addr + TIMER_OFST;

	/* initialize our peripheral timer hardware */
	io_result = ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
	io_result &= ALTERA_AVALON_TIMER_STATUS_TO_MSK |
	    ALTERA_AVALON_TIMER_STATUS_RUN_MSK;
	if (io_result != 0) {
		pr_err("peripheral timer hardware, incorrect initial state");
		goto bad_exit_iounmap;
	}

	period_100ms = (clk_rate / 10) - 1;
	iowrite32(period_100ms,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
	iowrite32(period_100ms >> 16,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));

	/* register our interrupt handler */
	ret_val = request_irq(g_demo_driver_irq,
			      demo_driver_interrupt_handler,
			      0,
			      the_platform_driver.driver.name,
			      &the_platform_driver);

	if (ret_val) {
		pr_err("request_irq failed");
		goto bad_exit_iounmap;
	}

	ret_val = -EBUSY;

	/* start our timer and enable our timer hardware interrupts */
	iowrite32(ALTERA_AVALON_TIMER_CONTROL_ITO_MSK |
		  ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
		  ALTERA_AVALON_TIMER_CONTROL_START_MSK,
		  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));

	io_result = ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
	io_result &= ALTERA_AVALON_TIMER_STATUS_RUN_MSK;
	if (io_result == 0) {
		/* stop our timer and disable our timer hardware interrupts */
		iowrite32(ALTERA_AVALON_TIMER_CONTROL_STOP_MSK,
			  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));

		pr_err("peripheral timer hardware, failed to start");
		goto bad_exit_freeirq;
	}

	g_platform_probe_flag = 1;
	up(&g_dev_probe_sem);
	pr_info("platform_probe exit\n");
	return 0;

bad_exit_freeirq:
	free_irq(g_demo_driver_irq, &the_platform_driver);
bad_exit_iounmap:
	iounmap(g_ioremap_addr);
bad_exit_release_mem_region:
	release_mem_region(g_demo_driver_base_addr, g_demo_driver_size);
bad_exit_return:
	up(&g_dev_probe_sem);
	pr_info("platform_probe bad_exit\n");
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	uint32_t io_result;
	pr_info("platform_remove enter\n");

	/* stop our timer and disable our timer hardware interrupts */
	iowrite32(ALTERA_AVALON_TIMER_CONTROL_STOP_MSK,
		  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));
	/* ensure there is no pending IRQ */
	do {
		io_result =
		    ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
		io_result &= ALTERA_AVALON_TIMER_STATUS_TO_MSK;
	} while (io_result != 0);

	free_irq(g_demo_driver_irq, &the_platform_driver);
	iounmap(g_ioremap_addr);
	release_mem_region(g_demo_driver_base_addr, g_demo_driver_size);

	if (down_interruptible(&g_dev_probe_sem))
		return -ERESTARTSYS;

	g_platform_probe_flag = 0;
	up(&g_dev_probe_sem);

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
		   .name = "demo_driver_5",
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

	sema_init(&g_dev_probe_sem, 1);

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
MODULE_DESCRIPTION("Demonstration Module 5 - reserve memory and enable IRQ");
MODULE_VERSION("1.0");
