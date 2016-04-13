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
static unsigned long g_demo_driver_clk_rate;
static void *g_ioremap_addr;
static void *g_timer_base;
static spinlock_t g_irq_lock;
static uint32_t g_max_irq_delay;
static uint32_t g_min_irq_delay;

static ssize_t interval_show(struct device_driver *driver, char *buf)
{
	unsigned long flags;
	uint32_t raw_status;
	uint32_t raw_periodl;
	uint32_t raw_periodh;
	uint32_t period;
	uint32_t interval;

	/* acquire the irq_lock */
	spin_lock_irqsave(&g_irq_lock, flags);

	/* capture the relevant hardware registers */
	raw_status = ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

	raw_periodl =
	    ioread32(IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));

	raw_periodh =
	    ioread32(IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));

	/* release the irq_lock */
	spin_unlock_irqrestore(&g_irq_lock, flags);

	/* calculate the current timer interval */
	raw_status &= ALTERA_AVALON_TIMER_STATUS_RUN_MSK;
	if (raw_status == 0) {
		interval = 0;
	} else {
		period = (raw_periodl & 0x0000FFFF) |
		    ((raw_periodh << 16) & 0xFFFF0000);
		period += 1;
		interval = g_demo_driver_clk_rate / period;
	}

	return scnprintf(buf, PAGE_SIZE,
			 "irq interval: %u per second\n", interval);
}

static ssize_t interval_store(struct device_driver *driver, const char *buf,
			      size_t count)
{
	int result;
	unsigned long flags;
	unsigned long new_interval;
	uint32_t new_period;
	uint32_t the_status;

	/* convert the input string to the requested new interval value */
	result = kstrtoul(buf, 0, &new_interval);
	if (result != 0)
		return -EINVAL;

	/* range check the requested new interval value */
	if (new_interval > 100)
		return -EINVAL;

	/* calculate the new period value */
	if (new_interval > 0)
		new_period = (g_demo_driver_clk_rate / new_interval) - 1;
	else
		new_period = g_demo_driver_clk_rate;

	/* acquire the irq_lock */
	spin_lock_irqsave(&g_irq_lock, flags);

	/* stop the interval timer */
	iowrite32(ALTERA_AVALON_TIMER_CONTROL_STOP_MSK,
		  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));

	/* ensure there is no pending IRQ that we are blocking */
	the_status = ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
	the_status &= ALTERA_AVALON_TIMER_STATUS_TO_MSK;
	if (the_status != 0) {
		do {
			/*
			   if we are blocking, release the lock to allow IRQ
			   handler to execute, acquire the lock and check again
			 */
			spin_unlock_irqrestore(&g_irq_lock, flags);
			spin_lock_irqsave(&g_irq_lock, flags);
			the_status =
			    ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS
				     (g_timer_base));
			the_status &= ALTERA_AVALON_TIMER_STATUS_TO_MSK;
		} while (the_status != 0);
	}

	/* write the new period value */
	iowrite32(new_period, IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
	iowrite32(new_period >> 16,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));

	/* initialize the MAX/MIN variables */
	g_max_irq_delay = 0;
	g_min_irq_delay = 0xFFFFFFFF;

	/* start the timer */
	if (new_interval > 0)
		iowrite32(ALTERA_AVALON_TIMER_CONTROL_ITO_MSK |
			  ALTERA_AVALON_TIMER_CONTROL_CONT_MSK |
			  ALTERA_AVALON_TIMER_CONTROL_START_MSK,
			  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));

	/* release the irq_lock */
	spin_unlock_irqrestore(&g_irq_lock, flags);
	return count;
}

DRIVER_ATTR(interval, (S_IRUGO|S_IWUSR|S_IWGRP), interval_show, interval_store);

static ssize_t irq_delays_show(struct device_driver *driver, char *buf)
{
	unsigned long flags;
	uint32_t max_irq_delay;
	uint32_t min_irq_delay;

	/* acquire the irq_lock */
	spin_lock_irqsave(&g_irq_lock, flags);

	/* capture the shared data values */
	max_irq_delay = g_max_irq_delay;
	min_irq_delay = g_min_irq_delay;

	/* release the irq_lock */
	spin_unlock_irqrestore(&g_irq_lock, flags);

	if (max_irq_delay == 0)
		return scnprintf(buf, PAGE_SIZE, "no IRQ delays yet\n");

	return scnprintf(buf, PAGE_SIZE,
			 "max: 0x%08X = %u\n"
			 "min: 0x%08X = %u\n",
			 max_irq_delay, max_irq_delay,
			 min_irq_delay, min_irq_delay);
}

DRIVER_ATTR(irq_delays, (S_IRUGO), irq_delays_show, NULL);

irqreturn_t demo_driver_interrupt_handler(int irq, void *dev_id)
{
	uint32_t raw_snapl;
	uint32_t raw_snaph;
	uint32_t snap;
	uint32_t raw_periodl;
	uint32_t raw_periodh;
	uint32_t period;
	uint32_t elapsed_ticks;

	spin_lock(&g_irq_lock);

	/* get the current timer value */
	iowrite32(0, IOADDR_ALTERA_AVALON_TIMER_SNAPL(g_timer_base));
	raw_snapl = ioread32(IOADDR_ALTERA_AVALON_TIMER_SNAPL(g_timer_base));
	raw_snaph = ioread32(IOADDR_ALTERA_AVALON_TIMER_SNAPH(g_timer_base));
	snap = (raw_snapl & 0x0000FFFF) | ((raw_snaph << 16) & 0xFFFF0000);

	/* get the current period value */
	raw_periodl =
	    ioread32(IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
	raw_periodh =
	    ioread32(IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));
	period = (raw_periodl & 0x0000FFFF) |
	    ((raw_periodh << 16) & 0xFFFF0000);

	/* calculate response delay and update MAX/MIN variables */
	elapsed_ticks = period - snap;
	if (elapsed_ticks > g_max_irq_delay)
		g_max_irq_delay = elapsed_ticks;
	if (elapsed_ticks < g_min_irq_delay)
		g_min_irq_delay = elapsed_ticks;

	/* clear the interrupt */
	iowrite32(0, IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

	spin_unlock(&g_irq_lock);
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

	g_demo_driver_clk_rate = clk_rate;

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

	period_100ms = (g_demo_driver_clk_rate / 10) - 1;
	iowrite32(period_100ms,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
	iowrite32(period_100ms >> 16,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));

	/* register our interrupt handler */
	spin_lock_init(&g_irq_lock);
	g_max_irq_delay = 0;
	g_min_irq_delay = 0xFFFFFFFF;

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

	/* create the sysfs entries */
	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_irq_delays);
	if (ret_val != 0) {
		pr_err("failed to create irq_delays sysfs entry");
		goto bad_exit_stop_timer;
	}

	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_interval);
	if (ret_val != 0) {
		pr_err("failed to create interval sysfs entry");
		goto bad_exit_remove_irq_delays;
	}

	g_platform_probe_flag = 1;
	up(&g_dev_probe_sem);
	pr_info("platform_probe exit\n");
	return 0;

bad_exit_remove_irq_delays:
	driver_remove_file(&the_platform_driver.driver,
			   &driver_attr_irq_delays);
bad_exit_stop_timer:
	iowrite32(ALTERA_AVALON_TIMER_CONTROL_STOP_MSK,
		  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));
	/* ensure there is no pending IRQ */
	do {
		io_result =
		    ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
		io_result &= ALTERA_AVALON_TIMER_STATUS_TO_MSK;
	} while (io_result != 0);
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

	driver_remove_file(&the_platform_driver.driver, &driver_attr_interval);
	driver_remove_file(&the_platform_driver.driver,
			   &driver_attr_irq_delays);

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
		   .name = "demo_driver_6",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
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
MODULE_DESCRIPTION("Demonstration Module 6 - introduce sysfs entries");
MODULE_VERSION("1.0");
