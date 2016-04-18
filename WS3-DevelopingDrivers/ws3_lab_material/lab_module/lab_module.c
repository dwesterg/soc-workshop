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
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/sched.h>

#include "lab_module.h"
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
static wait_queue_head_t g_irq_wait_queue;
static spinlock_t g_irq_lock;
static uint32_t g_irq_count;
static uint32_t g_max_irq_delay;
static uint32_t g_min_irq_delay;
void *g_preparser_strings[] = {
	"FILE=" __FILE__,
	"DATE=" __DATE__,
	"TIME=" __TIME__
};

/* misc device - lab_tmr_dev */
struct lab_tmr_dev {
	struct semaphore sem;
};

static struct lab_tmr_dev the_lab_tmr_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
};

static long
lab_tmr_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct lab_tmr_dev *dev = fp->private_data;
	unsigned long flags;
	uint32_t raw_status;
	uint32_t raw_periodl;
	uint32_t raw_periodh;
	uint32_t period;
	uint32_t interval;
	unsigned long new_interval;
	uint32_t new_period;
	uint32_t the_status;
	uint32_t max_irq_delay;
	uint32_t min_irq_delay;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	switch (cmd) {
	case IOC_SET_INTERVAL:
		if (get_user(new_interval, (uint32_t *)arg) < 0) {
			up(&dev->sem);
			return -EFAULT;
		}
		/* range check the requested new interval value */
		if (new_interval > 100) {
			up(&dev->sem);
			return -EINVAL;
		}
		/* calculate the new period value */
		if (new_interval > 0)
			new_period =
			    (g_demo_driver_clk_rate / new_interval) - 1;
		else
			new_period = g_demo_driver_clk_rate;

		/* acquire the irq_lock */
		spin_lock_irqsave(&g_irq_lock, flags);

		/* stop the interval timer */
		iowrite32(ALTERA_AVALON_TIMER_CONTROL_STOP_MSK,
			  IOADDR_ALTERA_AVALON_TIMER_CONTROL(g_timer_base));

		/* ensure there is no pending IRQ that we are blocking */
		the_status =
		    ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
		the_status &= ALTERA_AVALON_TIMER_STATUS_TO_MSK;
		if (the_status != 0) {
			do {
				/*
				   if we are blocking, release the lock to allow
				   IRQ handler to execute, acquire the lock and
				   check again
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
		iowrite32(new_period,
			  IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
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
				  IOADDR_ALTERA_AVALON_TIMER_CONTROL
				  (g_timer_base));

		/* release the irq_lock */
		spin_unlock_irqrestore(&g_irq_lock, flags);
		break;
	case IOC_GET_INTERVAL:
		/* acquire the irq_lock */
		spin_lock_irqsave(&g_irq_lock, flags);

		/* capture the relevant hardware registers */
		raw_status =
		    ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

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

		if (put_user(interval, (uint32_t *)arg) < 0) {
			up(&dev->sem);
			return -EFAULT;
		}
		break;
	case IOC_GET_MAX_DELAY:
		/* acquire the irq_lock */
		spin_lock_irqsave(&g_irq_lock, flags);

		/* capture the shared data values */
		max_irq_delay = g_max_irq_delay;

		/* release the irq_lock */
		spin_unlock_irqrestore(&g_irq_lock, flags);

		if (put_user(max_irq_delay, (uint32_t *)arg) < 0) {
			up(&dev->sem);
			return -EFAULT;
		}
		break;
	case IOC_GET_MIN_DELAY:
		/* acquire the irq_lock */
		spin_lock_irqsave(&g_irq_lock, flags);

		/* capture the shared data values */
		min_irq_delay = g_min_irq_delay;

		/* release the irq_lock */
		spin_unlock_irqrestore(&g_irq_lock, flags);

		if (put_user(min_irq_delay, (uint32_t *)arg) < 0) {
			up(&dev->sem);
			return -EFAULT;
		}
		break;
	default:
		up(&dev->sem);
		return -EINVAL;
	}

	up(&dev->sem);
	return 0;
}

static inline uint32_t get_current_irq_count(void)
{
	uint32_t current_count;
	unsigned long flags;

	spin_lock_irqsave(&g_irq_lock, flags);
	current_count = g_irq_count;
	spin_unlock_irqrestore(&g_irq_lock, flags);
	return current_count;
}

static ssize_t
lab_tmr_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct lab_tmr_dev *dev = fp->private_data;
	uint32_t cur_irq_cnt;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*offset != 0) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (count != 4) {
		up(&dev->sem);
		return -EINVAL;
	}

	cur_irq_cnt = get_current_irq_count();
	while (cur_irq_cnt == get_current_irq_count()) {
		up(&dev->sem);
		if (wait_event_interruptible(g_irq_wait_queue,
					     (cur_irq_cnt !=
					      get_current_irq_count())))
			return -ERESTARTSYS;

		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}

	cur_irq_cnt = get_current_irq_count();
	if (copy_to_user(user_buffer, &cur_irq_cnt, count)) {
		up(&dev->sem);
		return -EFAULT;
	}

	up(&dev->sem);
	return count;
}

static int lab_tmr_dev_open(struct inode *ip, struct file *fp)
{
	struct lab_tmr_dev *dev = &the_lab_tmr_dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	fp->private_data = dev;

	up(&dev->sem);
	return 0;
}

static int lab_tmr_dev_release(struct inode *ip, struct file *fp)
{
	struct lab_tmr_dev *dev = fp->private_data;

	if (down_interruptible(&dev->sem)) {
		return -ERESTARTSYS;
	}

	up(&dev->sem);
	return 0;
}

static const struct file_operations lab_tmr_dev_fops = {
	.owner = THIS_MODULE,
	.open = lab_tmr_dev_open,
	.release = lab_tmr_dev_release,
	.read = lab_tmr_dev_read,
	.unlocked_ioctl = lab_tmr_dev_ioctl,
};

static struct miscdevice lab_tmr_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lab_tmr",
	.fops = &lab_tmr_dev_fops,
};

/* misc device - lab_ram_dev */
struct lab_ram_dev {
	struct semaphore sem;
	unsigned char io_buf[IO_BUF_SIZE];
};

static struct lab_ram_dev the_lab_ram_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.io_buf = {0},
};

static loff_t lab_ram_dev_llseek(struct file *fp, loff_t offset, int mode)
{
	struct lab_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	switch (mode) {
	case SEEK_SET:
		next_offset = offset;
		break;
	case SEEK_CUR:
		next_offset = fp->f_pos + offset;
		break;
	case SEEK_END:
		next_offset = max_offset;
		break;
	default:
		up(&dev->sem);
		return -EINVAL;
	}

	if (next_offset < 0) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (next_offset > max_offset)
		next_offset = max_offset;

	fp->f_pos = next_offset;

	up(&dev->sem);
	return next_offset;
}

static ssize_t
lab_ram_dev_write(struct file *fp,
		   const char __user *user_buffer, size_t count,
		   loff_t *offset)
{
	struct lab_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *ram_ptr;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*offset > max_offset) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		return -ENOSPC;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	temp_count = count;
	ram_ptr = g_ioremap_addr + RAM_OFST;
	ram_ptr += *offset;

	while (temp_count > 0) {
		int this_loop_count = IO_BUF_SIZE;
		if (temp_count < IO_BUF_SIZE)
			this_loop_count = temp_count;

		if (copy_from_user
		    (&dev->io_buf, user_buffer, this_loop_count)) {
			up(&dev->sem);
			return -EFAULT;
		}
		memcpy_toio(ram_ptr, &dev->io_buf, this_loop_count);
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static ssize_t
lab_ram_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct lab_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *ram_ptr;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*offset > max_offset) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		return 0;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	temp_count = count;
	ram_ptr = g_ioremap_addr + RAM_OFST;
	ram_ptr += *offset;

	while (temp_count > 0) {
		int this_loop_count = IO_BUF_SIZE;
		if (temp_count < IO_BUF_SIZE)
			this_loop_count = temp_count;

		memcpy_fromio(&dev->io_buf, ram_ptr, this_loop_count);
		if (copy_to_user(user_buffer, &dev->io_buf, this_loop_count)) {
			up(&dev->sem);
			return -EFAULT;
		}
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static int lab_ram_dev_open(struct inode *ip, struct file *fp)
{
	struct lab_ram_dev *dev = &the_lab_ram_dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	fp->private_data = dev;

	up(&dev->sem);
	return 0;
}

static int lab_ram_dev_release(struct inode *ip, struct file *fp)
{
	struct lab_ram_dev *dev = fp->private_data;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	up(&dev->sem);
	return 0;
}

static const struct file_operations lab_ram_dev_fops = {
	.owner = THIS_MODULE,
	.open = lab_ram_dev_open,
	.release = lab_ram_dev_release,
	.read = lab_ram_dev_read,
	.write = lab_ram_dev_write,
	.llseek = lab_ram_dev_llseek,
};

static struct miscdevice lab_ram_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lab_ram",
	.fops = &lab_ram_dev_fops,
};

/* misc device - lab_rom_dev */
struct lab_rom_dev {
	struct semaphore sem;
	unsigned char io_buf[IO_BUF_SIZE];
};

static struct lab_rom_dev the_lab_rom_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.io_buf = {0},
};

static loff_t lab_rom_dev_llseek(struct file *fp, loff_t offset, int mode)
{
	struct lab_rom_dev *dev = fp->private_data;
	loff_t max_offset = ROM_SPAN;
	loff_t next_offset;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	switch (mode) {
	case SEEK_SET:
		next_offset = offset;
		break;
	case SEEK_CUR:
		next_offset = fp->f_pos + offset;
		break;
	case SEEK_END:
		next_offset = max_offset;
		break;
	default:
		up(&dev->sem);
		return -EINVAL;
	}

	if (next_offset < 0) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (next_offset > max_offset)
		next_offset = max_offset;

	fp->f_pos = next_offset;

	up(&dev->sem);
	return next_offset;
}

static ssize_t
lab_rom_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct lab_rom_dev *dev = fp->private_data;
	loff_t max_offset = ROM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *rom_ptr;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if (*offset > max_offset) {
		up(&dev->sem);
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		return 0;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	temp_count = count;
	rom_ptr = g_ioremap_addr + ROM_OFST;
	rom_ptr += *offset;

	while (temp_count > 0) {
		int this_loop_count = IO_BUF_SIZE;
		if (temp_count < IO_BUF_SIZE)
			this_loop_count = temp_count;

		memcpy_fromio(&dev->io_buf, rom_ptr, this_loop_count);
		if (copy_to_user(user_buffer, &dev->io_buf, this_loop_count)) {
			up(&dev->sem);
			return -EFAULT;
		}
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		rom_ptr += this_loop_count;
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static int lab_rom_dev_open(struct inode *ip, struct file *fp)
{
	struct lab_rom_dev *dev = &the_lab_rom_dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	fp->private_data = dev;

	up(&dev->sem);
	return 0;
}

static int lab_rom_dev_release(struct inode *ip, struct file *fp)
{
	struct lab_rom_dev *dev = fp->private_data;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	up(&dev->sem);
	return 0;
}

static const struct file_operations lab_rom_dev_fops = {
	.owner = THIS_MODULE,
	.open = lab_rom_dev_open,
	.release = lab_rom_dev_release,
	.read = lab_rom_dev_read,
	.llseek = lab_rom_dev_llseek,
};

static struct miscdevice lab_rom_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "lab_rom",
	.fops = &lab_rom_dev_fops,
};

/* platform driver */
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

static ssize_t
interval_store(struct device_driver *driver, const char *buf, size_t count)
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

DRIVER_ATTR(interval, (S_IWUSR | S_IWGRP | S_IRUGO), interval_show, interval_store);

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
	period =
	    (raw_periodl & 0x0000FFFF) | ((raw_periodh << 16) & 0xFFFF0000);

	/* calculate response delay and update MAX/MIN variables */
	elapsed_ticks = period - snap;
	if (elapsed_ticks > g_max_irq_delay)
		g_max_irq_delay = elapsed_ticks;
	if (elapsed_ticks < g_min_irq_delay)
		g_min_irq_delay = elapsed_ticks;

	/* clear the interrupt */
	iowrite32(0, IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));

	/* increment the IRQ count */
	g_irq_count++;

	spin_unlock(&g_irq_lock);
	wake_up_interruptible(&g_irq_wait_queue);
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

	ret_val = -EBUSY;

	/* acquire the probe lock */
	if (down_interruptible(&g_dev_probe_sem))
		return -ERESTARTSYS;

	if (g_platform_probe_flag != 0)
		goto bad_exit_return;

	ret_val = -EINVAL;

	/* get our first memory resource */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL)
		goto bad_exit_return;

	g_demo_driver_base_addr = r->start;
	g_demo_driver_size = resource_size(r);

	/* get our interrupt resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		goto bad_exit_return;

	g_demo_driver_irq = irq;

	/* get our clock resource */
	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
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
	if (demo_driver_mem_region == NULL)
		goto bad_exit_return;

	/* ioremap our memory region */
	g_ioremap_addr = ioremap(g_demo_driver_base_addr, g_demo_driver_size);
	if (g_ioremap_addr == NULL) {
		goto bad_exit_release_mem_region;
	}

	g_timer_base = g_ioremap_addr + TIMER_OFST;

	/* initialize our peripheral timer hardware */
	io_result = ioread32(IOADDR_ALTERA_AVALON_TIMER_STATUS(g_timer_base));
	io_result &= ALTERA_AVALON_TIMER_STATUS_TO_MSK |
	    ALTERA_AVALON_TIMER_STATUS_RUN_MSK;
	if (io_result != 0)
		goto bad_exit_iounmap;

	period_100ms = (g_demo_driver_clk_rate / 10) - 1;
	iowrite32(period_100ms,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODL(g_timer_base));
	iowrite32(period_100ms >> 16,
		  IOADDR_ALTERA_AVALON_TIMER_PERIODH(g_timer_base));

	/* register our interrupt handler */
	init_waitqueue_head(&g_irq_wait_queue);
	spin_lock_init(&g_irq_lock);
	g_irq_count = 0;
	g_max_irq_delay = 0;
	g_min_irq_delay = 0xFFFFFFFF;

	ret_val = request_irq(g_demo_driver_irq,
			      demo_driver_interrupt_handler,
			      0,
			      the_platform_driver.driver.name,
			      &the_platform_driver);

	if (ret_val)
		goto bad_exit_iounmap;

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

		goto bad_exit_freeirq;
	}
	/* create the sysfs entries */
	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_irq_delays);
	if (ret_val != 0)
		goto bad_exit_stop_timer;

	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_interval);
	if (ret_val != 0)
		goto bad_exit_remove_irq_delays;

	/* register misc device dev_rom */
	sema_init(&the_lab_rom_dev.sem, 1);
	ret_val = misc_register(&lab_rom_dev_device);
	if (ret_val != 0)
		goto bad_exit_remove_interval;

	/* register misc device dev_ram */
	sema_init(&the_lab_ram_dev.sem, 1);
	ret_val = misc_register(&lab_ram_dev_device);
	if (ret_val != 0)
		goto bad_exit_deregister_lab_rom;

	/* register misc device dev_tmr */
	sema_init(&the_lab_tmr_dev.sem, 1);
	ret_val = misc_register(&lab_tmr_dev_device);
	if (ret_val != 0)
		goto bad_exit_deregister_lab_ram;

	g_platform_probe_flag = 1;
	up(&g_dev_probe_sem);
	return 0;

bad_exit_deregister_lab_ram:
	misc_deregister(&lab_ram_dev_device);
bad_exit_deregister_lab_rom:
	misc_deregister(&lab_rom_dev_device);
bad_exit_remove_interval:
	driver_remove_file(&the_platform_driver.driver, &driver_attr_interval);
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
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	uint32_t io_result;

	misc_deregister(&lab_tmr_dev_device);
	misc_deregister(&lab_ram_dev_device);
	misc_deregister(&lab_rom_dev_device);

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
		   .name = "lab_driver",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
};

static int lab_init(void)
{
	int ret_val;

	sema_init(&g_dev_probe_sem, 1);

	ret_val = platform_driver_register(&the_platform_driver);
	if (ret_val != 0)
		return ret_val;

	return 0;
}

static void lab_exit(void)
{
	platform_driver_unregister(&the_platform_driver);
}

module_init(lab_init);
module_exit(lab_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lab Student One <lso@company.com>");
MODULE_DESCRIPTION("Lab Module - WS3 SoC Workshop");
MODULE_VERSION("1.0");
