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
static wait_queue_head_t g_irq_wait_queue;
static uint32_t g_irq_count;
static uint32_t g_max_irq_delay;
static uint32_t g_min_irq_delay;

/* misc device - demo_map_dev */
struct demo_map_dev {
	struct semaphore sem;
	unsigned long open_count;
	unsigned long release_count;
	unsigned long read_count;
	unsigned long mmap_count;
	unsigned long read_byte_count;
};

static struct demo_map_dev the_demo_map_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.open_count = 0,
	.release_count = 0,
	.read_count = 0,
	.mmap_count = 0,
	.read_byte_count = 0,
};

static int demo_map_dev_mmap(struct file *fp, struct vm_area_struct *vma)
{
	struct demo_map_dev *dev = fp->private_data;

	pr_info("demo_map_dev_mmap enter\n");
	pr_info(" fp = 0x%08X\n", (uint32_t)fp);
	pr_info("vma = 0x%08X\n", (uint32_t)vma);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_map_dev_mmap sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->mmap_count++;

	if (vma->vm_end - vma->vm_start != PAGE_SIZE) {
		up(&dev->sem);
		pr_info("demo_map_dev_mmap vma size not PAGE_SIZE exit\n");
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	vma->vm_pgoff = g_demo_driver_base_addr >> PAGE_SHIFT;

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, PAGE_SIZE,
			    vma->vm_page_prot)) {
		up(&dev->sem);
		pr_info("demo_map_dev_mmap remap_pfn_range exit\n");
		return -EAGAIN;
	}

	up(&dev->sem);
	pr_info("demo_map_dev_mmap exit\n");
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
demo_map_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct demo_map_dev *dev = fp->private_data;
	uint32_t cur_irq_cnt;

	pr_info("demo_map_dev_read enter\n");
	pr_info("         fp = 0x%08X\n", (uint32_t)fp);
	pr_info("user_buffer = 0x%08X\n", (uint32_t)user_buffer);
	pr_info("      count = 0x%08X\n", (uint32_t)count);
	pr_info("    *offset = 0x%08X\n", (uint32_t)*offset);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_map_dev_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->read_count++;

	if (*offset != 0) {
		up(&dev->sem);
		pr_info("demo_map_dev_read offset != 0 exit\n");
		return -EINVAL;
	}

	if (count != 4) {
		up(&dev->sem);
		pr_info("demo_map_dev_read count != 4 exit\n");
		return -EINVAL;
	}

	cur_irq_cnt = get_current_irq_count();
	while (cur_irq_cnt == get_current_irq_count()) {
		up(&dev->sem);
		if (wait_event_interruptible(g_irq_wait_queue,
					     (cur_irq_cnt !=
					      get_current_irq_count()))) {
			pr_info("demo_map_dev_read wait interrupted exit\n");
			return -ERESTARTSYS;
		}
		if (down_interruptible(&dev->sem)) {
			pr_info("demo_map_dev_read sem interrupted exit\n");
			return -ERESTARTSYS;
		}
	}

	cur_irq_cnt = get_current_irq_count();
	if (copy_to_user(user_buffer, &cur_irq_cnt, count)) {
		up(&dev->sem);
		pr_info("demo_map_dev_read copy_to_user exit\n");
		return -EFAULT;
	}

	dev->read_byte_count += count;

	up(&dev->sem);
	pr_info("demo_map_dev_read exit\n");
	return count;
}

static int demo_map_dev_open(struct inode *ip, struct file *fp)
{
	struct demo_map_dev *dev = &the_demo_map_dev;
	pr_info("demo_map_dev_open enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_map_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	fp->private_data = dev;
	dev->open_count++;

	up(&dev->sem);
	pr_info("demo_map_dev_open exit\n");
	return 0;
}

static int demo_map_dev_release(struct inode *ip, struct file *fp)
{
	struct demo_map_dev *dev = fp->private_data;
	pr_info("demo_map_dev_release enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_map_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->release_count++;

	up(&dev->sem);
	pr_info("demo_map_dev_release exit\n");
	return 0;
}

static const struct file_operations demo_map_dev_fops = {
	.owner = THIS_MODULE,
	.open = demo_map_dev_open,
	.release = demo_map_dev_release,
	.read = demo_map_dev_read,
	.mmap = demo_map_dev_mmap,
};

static struct miscdevice demo_map_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_map",
	.fops = &demo_map_dev_fops,
};

/* misc device - demo_tmr_dev */
struct demo_tmr_dev {
	struct semaphore sem;
	unsigned long open_count;
	unsigned long release_count;
	unsigned long ioctl_count;
};

static struct demo_tmr_dev the_demo_tmr_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.open_count = 0,
	.release_count = 0,
	.ioctl_count = 0,
};

static long
demo_tmr_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct demo_tmr_dev *dev = fp->private_data;
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

	pr_info("demo_tmr_dev_ioctl enter\n");
	pr_info(" fp = 0x%08X\n", (uint32_t)fp);
	pr_info("cmd = 0x%08X\n", (uint32_t)cmd);
	pr_info("arg = 0x%08X\n", (uint32_t)arg);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_tmr_dev_ioctl sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->ioctl_count++;

	switch (cmd) {
	case IOC_SET_INTERVAL:
		if (get_user(new_interval, (uint32_t *)arg) < 0) {
			up(&dev->sem);
			pr_info("demo_tmr_dev_ioctl get_user exit\n");
			return -EFAULT;
		}
		/* range check the requested new interval value */
		if (new_interval > 100) {
			up(&dev->sem);
			pr_info("demo_tmr_dev_ioctl new_interval > 100 exit\n");
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
			pr_info("demo_tmr_dev_ioctl put_user exit\n");
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
			pr_info("demo_tmr_dev_ioctl put_user exit\n");
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
			pr_info("demo_tmr_dev_ioctl put_user exit\n");
			return -EFAULT;
		}
		break;
	default:
		up(&dev->sem);
		pr_info("demo_tmr_dev_ioctl bad cmd exit\n");
		return -EINVAL;
	}

	up(&dev->sem);
	pr_info("demo_tmr_dev_ioctl exit\n");
	return 0;
}

static int demo_tmr_dev_open(struct inode *ip, struct file *fp)
{
	struct demo_tmr_dev *dev = &the_demo_tmr_dev;
	pr_info("demo_tmr_dev_open enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_tmr_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	fp->private_data = dev;
	dev->open_count++;

	up(&dev->sem);
	pr_info("demo_tmr_dev_open exit\n");
	return 0;
}

static int demo_tmr_dev_release(struct inode *ip, struct file *fp)
{
	struct demo_tmr_dev *dev = fp->private_data;
	pr_info("demo_tmr_dev_release enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_tmr_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->release_count++;

	up(&dev->sem);
	pr_info("demo_tmr_dev_release exit\n");
	return 0;
}

static const struct file_operations demo_tmr_dev_fops = {
	.owner = THIS_MODULE,
	.open = demo_tmr_dev_open,
	.release = demo_tmr_dev_release,
	.unlocked_ioctl = demo_tmr_dev_ioctl,
};

static struct miscdevice demo_tmr_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_tmr",
	.fops = &demo_tmr_dev_fops,
};

/* misc device - demo_ram_dev */
struct demo_ram_dev {
	struct semaphore sem;
	unsigned long open_count;
	unsigned long release_count;
	unsigned long read_count;
	unsigned long write_count;
	unsigned long llseek_count;
	unsigned long read_byte_count;
	unsigned long write_byte_count;
	unsigned char io_buf[IO_BUF_SIZE];
};

static struct demo_ram_dev the_demo_ram_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.open_count = 0,
	.release_count = 0,
	.read_count = 0,
	.write_count = 0,
	.llseek_count = 0,
	.read_byte_count = 0,
	.write_byte_count = 0,
	.io_buf = {0},
};

static loff_t demo_ram_dev_llseek(struct file *fp, loff_t offset, int mode)
{
	struct demo_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset;

	pr_info("demo_ram_dev_llseek enter\n");
	pr_info("    fp = 0x%08X\n", (uint32_t)fp);
	pr_info("offset = 0x%08X\n", (uint32_t)offset);
	pr_info("  mode = 0x%08X\n", (uint32_t)mode);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_ram_dev_llseek sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->llseek_count++;

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
		pr_info("demo_ram_dev_llseek bad mode exit\n");
		return -EINVAL;
	}

	if (next_offset < 0) {
		up(&dev->sem);
		pr_info("demo_ram_dev_llseek negative offset exit\n");
		return -EINVAL;
	}

	if (next_offset > max_offset)
		next_offset = max_offset;

	fp->f_pos = next_offset;

	up(&dev->sem);
	pr_info("demo_ram_dev_llseek exit\n");
	return next_offset;
}

static ssize_t
demo_ram_dev_write(struct file *fp,
		   const char __user *user_buffer, size_t count,
		   loff_t *offset)
{
	struct demo_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *ram_ptr;

	pr_info("demo_ram_dev_write enter\n");
	pr_info("         fp = 0x%08X\n", (uint32_t)fp);
	pr_info("user_buffer = 0x%08X\n", (uint32_t)user_buffer);
	pr_info("      count = 0x%08X\n", (uint32_t)count);
	pr_info("    *offset = 0x%08X\n", (uint32_t)*offset);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_ram_dev_write sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->write_count++;

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_ram_dev_write offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_ram_dev_write offset == max_offset exit\n");
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
			pr_info("demo_ram_dev_write copy_from_user exit\n");
			return -EFAULT;
		}
		memcpy_toio(ram_ptr, &dev->io_buf, this_loop_count);
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
	}

	dev->write_byte_count += count;
	*offset += count;

	up(&dev->sem);
	pr_info("demo_ram_dev_write exit\n");
	return count;
}

static ssize_t
demo_ram_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct demo_ram_dev *dev = fp->private_data;
	loff_t max_offset = RAM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *ram_ptr;

	pr_info("demo_ram_dev_read enter\n");
	pr_info("         fp = 0x%08X\n", (uint32_t)fp);
	pr_info("user_buffer = 0x%08X\n", (uint32_t)user_buffer);
	pr_info("      count = 0x%08X\n", (uint32_t)count);
	pr_info("    *offset = 0x%08X\n", (uint32_t)*offset);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_ram_dev_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->read_count++;

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_ram_dev_read offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_ram_dev_read offset == max_offset exit\n");
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
			pr_info("demo_ram_dev_read copy_to_user exit\n");
			return -EFAULT;
		}
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
	}

	dev->read_byte_count += count;
	*offset += count;

	up(&dev->sem);
	pr_info("demo_ram_dev_read exit\n");
	return count;
}

static int demo_ram_dev_open(struct inode *ip, struct file *fp)
{
	struct demo_ram_dev *dev = &the_demo_ram_dev;
	pr_info("demo_ram_dev_open enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_ram_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	fp->private_data = dev;
	dev->open_count++;

	up(&dev->sem);
	pr_info("demo_ram_dev_open exit\n");
	return 0;
}

static int demo_ram_dev_release(struct inode *ip, struct file *fp)
{
	struct demo_ram_dev *dev = fp->private_data;
	pr_info("demo_ram_dev_release enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_ram_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->release_count++;

	up(&dev->sem);
	pr_info("demo_ram_dev_release exit\n");
	return 0;
}

static const struct file_operations demo_ram_dev_fops = {
	.owner = THIS_MODULE,
	.open = demo_ram_dev_open,
	.release = demo_ram_dev_release,
	.read = demo_ram_dev_read,
	.write = demo_ram_dev_write,
	.llseek = demo_ram_dev_llseek,
};

static struct miscdevice demo_ram_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_ram",
	.fops = &demo_ram_dev_fops,
};

/* misc device - demo_rom_dev */
struct demo_rom_dev {
	struct semaphore sem;
	unsigned long open_count;
	unsigned long release_count;
	unsigned long read_count;
	unsigned long llseek_count;
	unsigned long read_byte_count;
	unsigned char io_buf[IO_BUF_SIZE];
};

static struct demo_rom_dev the_demo_rom_dev = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.open_count = 0,
	.release_count = 0,
	.read_count = 0,
	.llseek_count = 0,
	.read_byte_count = 0,
	.io_buf = {0},
};

static loff_t demo_rom_dev_llseek(struct file *fp, loff_t offset, int mode)
{
	struct demo_rom_dev *dev = fp->private_data;
	loff_t max_offset = ROM_SPAN;
	loff_t next_offset;

	pr_info("demo_rom_dev_llseek enter\n");
	pr_info("    fp = 0x%08X\n", (uint32_t)fp);
	pr_info("offset = 0x%08X\n", (uint32_t)offset);
	pr_info("  mode = 0x%08X\n", (uint32_t)mode);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_rom_dev_llseek sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->llseek_count++;

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
		pr_info("demo_rom_dev_llseek bad mode exit\n");
		return -EINVAL;
	}

	if (next_offset < 0) {
		up(&dev->sem);
		pr_info("demo_rom_dev_llseek negative offset exit\n");
		return -EINVAL;
	}

	if (next_offset > max_offset)
		next_offset = max_offset;

	fp->f_pos = next_offset;

	up(&dev->sem);
	pr_info("demo_rom_dev_llseek exit\n");
	return next_offset;
}

static ssize_t
demo_rom_dev_read(struct file *fp, char __user *user_buffer,
		  size_t count, loff_t *offset)
{
	struct demo_rom_dev *dev = fp->private_data;
	loff_t max_offset = ROM_SPAN;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	void *rom_ptr;

	pr_info("demo_rom_dev_read enter\n");
	pr_info("         fp = 0x%08X\n", (uint32_t)fp);
	pr_info("user_buffer = 0x%08X\n", (uint32_t)user_buffer);
	pr_info("      count = 0x%08X\n", (uint32_t)count);
	pr_info("    *offset = 0x%08X\n", (uint32_t)*offset);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_rom_dev_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->read_count++;

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_rom_dev_read offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_rom_dev_read offset == max_offset exit\n");
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
			pr_info("demo_rom_dev_read copy_to_user exit\n");
			return -EFAULT;
		}
		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		rom_ptr += this_loop_count;
	}

	dev->read_byte_count += count;
	*offset += count;

	up(&dev->sem);
	pr_info("demo_rom_dev_read exit\n");
	return count;
}

static int demo_rom_dev_open(struct inode *ip, struct file *fp)
{
	struct demo_rom_dev *dev = &the_demo_rom_dev;
	pr_info("demo_rom_dev_open enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_rom_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	fp->private_data = dev;
	dev->open_count++;

	up(&dev->sem);
	pr_info("demo_rom_dev_open exit\n");
	return 0;
}

static int demo_rom_dev_release(struct inode *ip, struct file *fp)
{
	struct demo_rom_dev *dev = fp->private_data;
	pr_info("demo_rom_dev_release enter\n");
	pr_info("ip = 0x%08X\n", (uint32_t)ip);
	pr_info("fp = 0x%08X\n", (uint32_t)fp);

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_rom_dev_open sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	dev->release_count++;

	up(&dev->sem);
	pr_info("demo_rom_dev_release exit\n");
	return 0;
}

static const struct file_operations demo_rom_dev_fops = {
	.owner = THIS_MODULE,
	.open = demo_rom_dev_open,
	.release = demo_rom_dev_release,
	.read = demo_rom_dev_read,
	.llseek = demo_rom_dev_llseek,
};

static struct miscdevice demo_rom_dev_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_rom",
	.fops = &demo_rom_dev_fops,
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

DRIVER_ATTR(interval, (S_IWUSR|S_IWGRP|S_IRUGO), interval_show, interval_store);

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
	/* register misc device dev_rom */
	sema_init(&the_demo_rom_dev.sem, 1);
	ret_val = misc_register(&demo_rom_dev_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_rom\"...");
		goto bad_exit_remove_interval;
	}
	/* register misc device dev_ram */
	sema_init(&the_demo_ram_dev.sem, 1);
	ret_val = misc_register(&demo_ram_dev_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_ram\"...");
		goto bad_exit_deregister_demo_rom;
	}
	/* register misc device dev_tmr */
	sema_init(&the_demo_tmr_dev.sem, 1);
	ret_val = misc_register(&demo_tmr_dev_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_tmr\"...");
		goto bad_exit_deregister_demo_ram;
	}
	/* register misc device dev_map */
	sema_init(&the_demo_map_dev.sem, 1);
	ret_val = misc_register(&demo_map_dev_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_map\"...");
		goto bad_exit_deregister_demo_tmr;
	}

	g_platform_probe_flag = 1;
	up(&g_dev_probe_sem);
	pr_info("platform_probe exit\n");
	return 0;

bad_exit_deregister_demo_tmr:
	misc_deregister(&demo_tmr_dev_device);
bad_exit_deregister_demo_ram:
	misc_deregister(&demo_ram_dev_device);
bad_exit_deregister_demo_rom:
	misc_deregister(&demo_rom_dev_device);
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
	pr_info("platform_probe bad_exit\n");
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	uint32_t io_result;
	pr_info("platform_remove enter\n");

	misc_deregister(&demo_map_dev_device);
	misc_deregister(&demo_tmr_dev_device);
	misc_deregister(&demo_ram_dev_device);
	misc_deregister(&demo_rom_dev_device);

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

	pr_info("\n");
	pr_info("demo_rom_dev stats:\n");
	pr_info("      open_count = %lu\n", the_demo_rom_dev.open_count);
	pr_info("   release_count = %lu\n", the_demo_rom_dev.release_count);
	pr_info("      read_count = %lu\n", the_demo_rom_dev.read_count);
	pr_info("    llseek_count = %lu\n", the_demo_rom_dev.llseek_count);
	pr_info(" read_byte_count = %lu\n", the_demo_rom_dev.read_byte_count);
	pr_info("\n");
	pr_info("demo_ram_dev stats:\n");
	pr_info("      open_count = %lu\n", the_demo_ram_dev.open_count);
	pr_info("   release_count = %lu\n", the_demo_ram_dev.release_count);
	pr_info("      read_count = %lu\n", the_demo_ram_dev.read_count);
	pr_info("     write_count = %lu\n", the_demo_ram_dev.write_count);
	pr_info("    llseek_count = %lu\n", the_demo_ram_dev.llseek_count);
	pr_info(" read_byte_count = %lu\n", the_demo_ram_dev.read_byte_count);
	pr_info("write_byte_count = %lu\n", the_demo_ram_dev.write_byte_count);
	pr_info("\n");
	pr_info("demo_tmr_dev stats:\n");
	pr_info("      open_count = %lu\n", the_demo_tmr_dev.open_count);
	pr_info("   release_count = %lu\n", the_demo_tmr_dev.release_count);
	pr_info("     ioctl_count = %lu\n", the_demo_tmr_dev.ioctl_count);
	pr_info("\n");
	pr_info("demo_map_dev stats:\n");
	pr_info("      open_count = %lu\n", the_demo_map_dev.open_count);
	pr_info("   release_count = %lu\n", the_demo_map_dev.release_count);
	pr_info("      read_count = %lu\n", the_demo_map_dev.read_count);
	pr_info("      mmap_count = %lu\n", the_demo_map_dev.mmap_count);
	pr_info(" read_byte_count = %lu\n", the_demo_map_dev.read_byte_count);
	pr_info("\n");

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
		   .name = "demo_driver_7",
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
MODULE_DESCRIPTION("Demonstration Module 7 - introduce misc device");
MODULE_VERSION("1.0");
