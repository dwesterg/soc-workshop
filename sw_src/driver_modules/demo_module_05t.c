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

#include "demo_module.h"
#include "my_altera_avalon_timer_regs.h"

/* globals */
static int g_demo_driver_base_addr;
static int g_demo_driver_irq;
static void __iomem *g_ioremap_addr;
static void __iomem *g_timer_base;

/* parameters */
static unsigned int base = 0xFFFFFFFF;
static unsigned int irq = 0xFFFFFFFF;
module_param(base, uint, S_IRUGO);
module_param(irq, uint, S_IRUGO);
MODULE_PARM_DESC(base, "the base address to request");
MODULE_PARM_DESC(irq, "the IRQ to register");

irqreturn_t demo_driver_interrupt_handler(int irq, void *dev_id)
{
	/* clear the interrupt */
	iowrite32(0, IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

	return IRQ_HANDLED;
}

static int demo_init(void)
{
	int ret_val;
	struct resource *demo_driver_mem_region;
	int undo_request_mem_region = 0;
	int undo_ioremap = 0;
	int undo_request_irq = 0;

	pr_info("demo_init enter\n");

	if (base == 0xFFFFFFFF) {
		pr_err("must provide parameter 'base'\n");
		return -EINVAL;
	}

	if (irq == 0xFFFFFFFF) {
		pr_err("must provide parameter 'irq'\n");
		return -EINVAL;
	}

	g_demo_driver_base_addr = base;
	g_demo_driver_irq = irq;

	/* reserve our memory region */
	demo_driver_mem_region = request_mem_region(g_demo_driver_base_addr,
						    PAGE_SIZE,
						    "demo_driver_test_hw_region");
	if (demo_driver_mem_region == NULL) {
		pr_err("request_mem_region failed: g_demo_driver_base_addr\n");
	} else {
		pr_info("request_mem_region succeeded\n");
		undo_request_mem_region = 1;
	}

	/* ioremap our memory region */
	g_ioremap_addr = ioremap(g_demo_driver_base_addr, PAGE_SIZE);
	if (g_ioremap_addr == NULL) {
		pr_err("ioremap failed: g_demo_driver_base_addr\n");
	} else {
		pr_info("ioremap succeeded\n");
		undo_ioremap = 1;
	}

	g_timer_base = g_ioremap_addr + TIMER_OFST;

	/* register our interrupt handler */
	ret_val = request_irq(g_demo_driver_irq,
			      demo_driver_interrupt_handler,
			      0,
			      "demo_driver_test",
			      demo_driver_interrupt_handler);

	if (ret_val) {
		pr_err("request_irq failed\n");
	} else {
		pr_info("request_irq succeeded\n");
		undo_request_irq = 1;
	}

	if (undo_request_irq != 0)
		free_irq(g_demo_driver_irq, demo_driver_interrupt_handler);
	if (undo_ioremap != 0)
		iounmap(g_ioremap_addr);
	if (undo_request_mem_region != 0)
		release_mem_region(g_demo_driver_base_addr, PAGE_SIZE);

	pr_info("demo_init exit\n");
	return -EBUSY;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");
	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_DESCRIPTION("Demonstration Module 5t - test memory reservation and IRQ");
MODULE_VERSION("1.0");
