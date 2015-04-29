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
#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include "demo_module.h"
#include "my_altera_msgdma_descriptor_regs.h"
#include "my_altera_msgdma_csr_regs.h"

/* prototypes */
static struct platform_driver the_platform_driver;

/* globals */
static struct semaphore g_dev_probe_sem;
static int g_platform_probe_flag;
static int g_demo_dma_csr_addr;
static int g_demo_dma_csr_size;
static int g_demo_dma_desc_addr;
static int g_demo_dma_desc_size;
static int g_demo_dma_irq;
static void *g_ioremap_csr_addr;
static void *g_ioremap_desc_addr;
static spinlock_t g_irq_lock;
static wait_queue_head_t g_irq_wait_queue;
static uint32_t g_irq_count;
static uint32_t g_max_irq_delay;
static uint32_t g_min_irq_delay;
static void *g_kmalloc_ptr_4k;
static void *g_coherent_ptr_4k;
static dma_addr_t g_dma_handle_4k;
static void *g_coherent_ptr_1m;
static dma_addr_t g_dma_handle_1m;

/* common across both devices */
struct demo_dma_xx {
	struct semaphore sem;
	struct device *pdev_dev;
};

static struct demo_dma_xx the_demo_dma_xx = {
	/*
	   .sem = initialize this at runtime before it is needed
	 */
	.pdev_dev = NULL,
};

static loff_t demo_dma_xx_llseek(struct file *fp, loff_t offset, int mode)
{
	struct demo_dma_xx *dev = &the_demo_dma_xx;
	loff_t max_offset = DMA_DEVICE_BUFFER_SIZE;
	loff_t next_offset;

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_dma_xx_llseek sem interrupted exit\n");
		return -ERESTARTSYS;
	}

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
		pr_info("demo_dma_xx_llseek bad mode exit\n");
		return -EINVAL;
	}

	if (next_offset < 0) {
		up(&dev->sem);
		pr_info("demo_dma_xx_llseek negative offset exit\n");
		return -EINVAL;
	}

	if (next_offset & DMA_DEVICE_MIN_BLOCK_MASK) {
		up(&dev->sem);
		pr_info("demo_dma_xx_llseek offset not min multiple exit\n");
		return -EINVAL;
	}

	if (next_offset > max_offset)
		next_offset = max_offset;

	fp->f_pos = next_offset;

	up(&dev->sem);
	return next_offset;
}

static inline uint32_t get_dma_fill_level(void)
{
	uint32_t dma_fill_level;
	uint32_t dma_read_level;
	uint32_t dma_write_level;
	uint32_t dma_max_level;

	dma_fill_level = ioread32(g_ioremap_csr_addr +
				  CSR_DESCRIPTOR_FILL_LEVEL_REG);

	dma_read_level = dma_fill_level;
	dma_read_level &= ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_MASK;
	dma_read_level >>= ALTERA_MSGDMA_CSR_READ_FILL_LEVEL_OFFSET;

	dma_write_level = dma_fill_level;
	dma_write_level &= ALTERA_MSGDMA_CSR_WRITE_FILL_LEVEL_MASK;
	dma_write_level >>= ALTERA_MSGDMA_CSR_WRITE_FILL_LEVEL_OFFSET;

	dma_max_level = (dma_write_level > dma_read_level) ?
	    (dma_write_level) : (dma_read_level);

	return dma_max_level;
}

static inline uint32_t get_dma_busy(void)
{
	uint32_t dma_status;

	dma_status = ioread32(g_ioremap_csr_addr + CSR_STATUS_REG);
	dma_status &= ALTERA_MSGDMA_CSR_BUSY_MASK;
	return dma_status;
}

/* misc device - demo_dma_st */
static ssize_t demo_dma_st_write(struct file *fp,
				 const char __user *user_buffer, size_t count,
				 loff_t *offset)
{
	struct demo_dma_xx *dev = &the_demo_dma_xx;
	loff_t max_offset = DMA_DEVICE_BUFFER_SIZE;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	uint32_t ram_ptr;
	uint32_t next_io_buf_ofst;
	int this_loop_count = DMA_DEVICE_MIN_BLOCK_SIZE;
	dma_addr_t dma_handle = 0;
	dma_addr_t last_dma_handle_0 = 0;
	dma_addr_t last_dma_handle_1 = 0;
	dma_addr_t last_dma_handle_2 = 0;
	dma_addr_t last_dma_handle_3 = 0;

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_dma_st_write sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_st_write offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_st_write offset == max_offset exit\n");
		return -ENOSPC;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	if (count & DMA_DEVICE_MIN_BLOCK_MASK) {
		up(&dev->sem);
		pr_info("demo_dma_st_write count not min multiple exit\n");
		return -EINVAL;
	}

	temp_count = count;
	ram_ptr = g_dma_handle_1m;
	ram_ptr += *offset;
	next_io_buf_ofst = 0;

	while (temp_count > 0) {
		while (get_dma_fill_level() > 2) {
			if (wait_event_interruptible(g_irq_wait_queue,
						     (get_dma_fill_level() <=
						      2))) {
				up(&dev->sem);
				pr_info
				  ("demo_dma_st_write wait interrupted exit\n");
				return -ERESTARTSYS;
			}
		}

		if (last_dma_handle_3 != 0) {
			dma_unmap_single(dev->pdev_dev, last_dma_handle_3,
					 this_loop_count, DMA_TO_DEVICE);
			last_dma_handle_3 = 0;
		}

		if (copy_from_user(g_kmalloc_ptr_4k +
				   next_io_buf_ofst,
				   user_buffer, this_loop_count)) {
			up(&dev->sem);
			pr_info("demo_dma_st_write copy_from_user exit\n");
			return -EFAULT;
		}

		dma_handle = dma_map_single(dev->pdev_dev,
					    g_kmalloc_ptr_4k + next_io_buf_ofst,
					    this_loop_count, DMA_TO_DEVICE);

		if (dma_mapping_error(dev->pdev_dev, dma_handle)) {
			up(&dev->sem);
			pr_info("demo_dma_st_write dma mapping error exit\n");
			return -EBUSY;
		}

		last_dma_handle_3 = last_dma_handle_2;
		last_dma_handle_2 = last_dma_handle_1;
		last_dma_handle_1 = last_dma_handle_0;
		last_dma_handle_0 = dma_handle;

		iowrite32(dma_handle,
			  g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
		iowrite32(ram_ptr,
			  g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
		iowrite32(this_loop_count,
			  g_ioremap_desc_addr + DESC_LENGTH_REG);
		iowrite32(START_DMA_MASK,
			  g_ioremap_desc_addr + DESC_CONTROL_REG);

		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
		next_io_buf_ofst += this_loop_count;
		if (next_io_buf_ofst >= PAGE_SIZE)
			next_io_buf_ofst = 0;
	}

	while (get_dma_busy() != 0) {
		if (wait_event_interruptible(g_irq_wait_queue,
					     (get_dma_busy() == 0))) {
			up(&dev->sem);
			pr_info("demo_dma_st_write wait interrupted exit\n");
			return -ERESTARTSYS;
		}
	}

	if (last_dma_handle_3 != 0)
		dma_unmap_single(dev->pdev_dev, last_dma_handle_3,
				 this_loop_count, DMA_TO_DEVICE);

	if (last_dma_handle_2 != 0)
		dma_unmap_single(dev->pdev_dev, last_dma_handle_2,
				 this_loop_count, DMA_TO_DEVICE);

	if (last_dma_handle_1 != 0)
		dma_unmap_single(dev->pdev_dev, last_dma_handle_1,
				 this_loop_count, DMA_TO_DEVICE);

	if (last_dma_handle_0 != 0)
		dma_unmap_single(dev->pdev_dev, last_dma_handle_0,
				 this_loop_count, DMA_TO_DEVICE);

	*offset += count;

	up(&dev->sem);
	return count;
}

static ssize_t demo_dma_st_read(struct file *fp, char __user *user_buffer,
				size_t count, loff_t *offset)
{
	struct demo_dma_xx *dev = &the_demo_dma_xx;
	loff_t max_offset = DMA_DEVICE_BUFFER_SIZE;
	loff_t next_offset = *offset + count;
	size_t temp_user_count;
	size_t temp_dma_count;
	uint32_t ram_ptr;
	uint32_t next_dma_io_buf_ofst;
	uint32_t next_user_io_buf_ofst;
	int this_loop_count;
	dma_addr_t dma_handle = 0;
	dma_addr_t last_dma_handle_0 = 0;
	dma_addr_t last_dma_handle_1 = 0;

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_dma_st_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_st_read offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_st_read offset == max_offset exit\n");
		return 0;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	if (count & DMA_DEVICE_MIN_BLOCK_MASK) {
		up(&dev->sem);
		pr_info("demo_dma_st_read count not min multiple exit\n");
		return -EINVAL;
	}

	this_loop_count = DMA_DEVICE_MIN_BLOCK_SIZE;
	temp_user_count = count;
	temp_dma_count = count;
	ram_ptr = g_dma_handle_1m;
	ram_ptr += *offset;
	next_dma_io_buf_ofst = 0;
	next_user_io_buf_ofst = 0;

	dma_handle = dma_map_single(dev->pdev_dev,
				    g_kmalloc_ptr_4k + next_dma_io_buf_ofst,
				    this_loop_count, DMA_FROM_DEVICE);

	if (dma_mapping_error(dev->pdev_dev, dma_handle)) {
		up(&dev->sem);
		pr_info("demo_dma_st_read dma mapping error exit\n");
		return -EBUSY;
	}

	last_dma_handle_1 = last_dma_handle_0;
	last_dma_handle_0 = dma_handle;

	iowrite32(ram_ptr, g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
	iowrite32(dma_handle, g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
	iowrite32(this_loop_count, g_ioremap_desc_addr + DESC_LENGTH_REG);
	iowrite32(START_DMA_MASK, g_ioremap_desc_addr + DESC_CONTROL_REG);

	temp_dma_count -= this_loop_count;
	ram_ptr += this_loop_count;
	next_dma_io_buf_ofst += this_loop_count;
	if (next_dma_io_buf_ofst >= PAGE_SIZE)
		next_dma_io_buf_ofst = 0;

	while (temp_user_count > 0) {
		if (temp_dma_count > 0) {
			dma_handle = dma_map_single(dev->pdev_dev,
						    g_kmalloc_ptr_4k +
						    next_dma_io_buf_ofst,
						    this_loop_count,
						    DMA_FROM_DEVICE);

			if (dma_mapping_error(dev->pdev_dev, dma_handle)) {
				up(&dev->sem);
				pr_info
				  ("demo_dma_st_read dma mapping error exit\n");
				return -EBUSY;
			}

			last_dma_handle_1 = last_dma_handle_0;
			last_dma_handle_0 = dma_handle;

			iowrite32(ram_ptr,
				  g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
			iowrite32(dma_handle,
				  g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
			iowrite32(this_loop_count,
				  g_ioremap_desc_addr + DESC_LENGTH_REG);
			iowrite32(START_DMA_MASK,
				  g_ioremap_desc_addr + DESC_CONTROL_REG);

			temp_dma_count -= this_loop_count;
			ram_ptr += this_loop_count;
			next_dma_io_buf_ofst += this_loop_count;
			if (next_dma_io_buf_ofst >= PAGE_SIZE)
				next_dma_io_buf_ofst = 0;
		} else {
			while (get_dma_busy() != 0) {
				if (wait_event_interruptible(g_irq_wait_queue,
							     (get_dma_busy() ==
							      0))) {
					up(&dev->sem);
					pr_info
				   ("demo_dma_st_read wait interrupted exit\n");
					return -ERESTARTSYS;
				}
			}
		}

		while (get_dma_fill_level() > 0) {
			if (wait_event_interruptible(g_irq_wait_queue,
						     (get_dma_fill_level() ==
						      0))) {
				up(&dev->sem);
				pr_info
				   ("demo_dma_st_read wait interrupted exit\n");
				return -ERESTARTSYS;
			}
		}

		if (last_dma_handle_1 != 0) {
			dma_unmap_single(dev->pdev_dev, last_dma_handle_1,
					 this_loop_count, DMA_FROM_DEVICE);
			last_dma_handle_1 = 0;
		} else if (last_dma_handle_0 != 0) {
			dma_unmap_single(dev->pdev_dev, last_dma_handle_0,
					 this_loop_count, DMA_FROM_DEVICE);
			last_dma_handle_0 = 0;
		}

		if (copy_to_user(user_buffer, g_kmalloc_ptr_4k +
				 next_user_io_buf_ofst, this_loop_count)) {
			up(&dev->sem);
			pr_info("demo_dma_st_read copy_to_user exit\n");
			return -EFAULT;
		}
		temp_user_count -= this_loop_count;
		user_buffer += this_loop_count;
		next_user_io_buf_ofst += this_loop_count;
		if (next_user_io_buf_ofst >= PAGE_SIZE)
			next_user_io_buf_ofst = 0;
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static const struct file_operations demo_dma_st_fops = {
	.owner = THIS_MODULE,
	.read = demo_dma_st_read,
	.write = demo_dma_st_write,
	.llseek = demo_dma_xx_llseek,
};

static struct miscdevice demo_dma_st_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_dma_st",
	.fops = &demo_dma_st_fops,
};

/* misc device - demo_dma_co */
static ssize_t demo_dma_co_write(struct file *fp,
				 const char __user *user_buffer, size_t count,
				 loff_t *offset)
{
	struct demo_dma_xx *dev = &the_demo_dma_xx;
	loff_t max_offset = DMA_DEVICE_BUFFER_SIZE;
	loff_t next_offset = *offset + count;
	size_t temp_count;
	uint32_t ram_ptr;
	uint32_t next_io_buf_ofst;
	int this_loop_count = DMA_DEVICE_MIN_BLOCK_SIZE;

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_dma_co_write sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_co_write offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_co_write offset == max_offset exit\n");
		return -ENOSPC;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	if (count & DMA_DEVICE_MIN_BLOCK_MASK) {
		up(&dev->sem);
		pr_info("demo_dma_co_write count not min multiple exit\n");
		return -EINVAL;
	}

	temp_count = count;
	ram_ptr = g_dma_handle_1m;
	ram_ptr += *offset;
	next_io_buf_ofst = 0;

	while (temp_count > 0) {
		while (get_dma_fill_level() > 2) {
			if (wait_event_interruptible(g_irq_wait_queue,
						     (get_dma_fill_level() <=
						      2))) {
				up(&dev->sem);
				pr_info
				  ("demo_dma_co_write wait interrupted exit\n");
				return -ERESTARTSYS;
			}
		}

		if (copy_from_user(g_coherent_ptr_4k +
				   next_io_buf_ofst,
				   user_buffer, this_loop_count)) {
			up(&dev->sem);
			pr_info("demo_dma_co_write copy_from_user exit\n");
			return -EFAULT;
		}

		iowrite32(g_dma_handle_4k + next_io_buf_ofst,
			  g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
		iowrite32(ram_ptr,
			  g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
		iowrite32(this_loop_count,
			  g_ioremap_desc_addr + DESC_LENGTH_REG);
		iowrite32(START_DMA_MASK,
			  g_ioremap_desc_addr + DESC_CONTROL_REG);

		temp_count -= this_loop_count;
		user_buffer += this_loop_count;
		ram_ptr += this_loop_count;
		next_io_buf_ofst += this_loop_count;
		if (next_io_buf_ofst >= PAGE_SIZE)
			next_io_buf_ofst = 0;
	}

	while (get_dma_busy() != 0) {
		if (wait_event_interruptible(g_irq_wait_queue,
					     (get_dma_busy() == 0))) {
			up(&dev->sem);
			pr_info("demo_dma_co_write wait interrupted exit\n");
			return -ERESTARTSYS;
		}
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static ssize_t demo_dma_co_read(struct file *fp, char __user *user_buffer,
				size_t count, loff_t *offset)
{
	struct demo_dma_xx *dev = &the_demo_dma_xx;
	loff_t max_offset = DMA_DEVICE_BUFFER_SIZE;
	loff_t next_offset = *offset + count;
	size_t temp_user_count;
	size_t temp_dma_count;
	uint32_t ram_ptr;
	uint32_t next_dma_io_buf_ofst;
	uint32_t next_user_io_buf_ofst;
	int this_loop_count;

	if (down_interruptible(&dev->sem)) {
		pr_info("demo_dma_co_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (*offset > max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_co_read offset > max_offset exit\n");
		return -EINVAL;
	}

	if (*offset == max_offset) {
		up(&dev->sem);
		pr_info("demo_dma_co_read offset == max_offset exit\n");
		return 0;
	}

	if (next_offset > max_offset)
		count -= next_offset - max_offset;

	if (count & DMA_DEVICE_MIN_BLOCK_MASK) {
		up(&dev->sem);
		pr_info("demo_dma_co_read count not min multiple exit\n");
		return -EINVAL;
	}

	this_loop_count = DMA_DEVICE_MIN_BLOCK_SIZE;
	temp_user_count = count;
	temp_dma_count = count;
	ram_ptr = g_dma_handle_1m;
	ram_ptr += *offset;
	next_dma_io_buf_ofst = 0;
	next_user_io_buf_ofst = 0;

	iowrite32(ram_ptr, g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
	iowrite32(g_dma_handle_4k + next_dma_io_buf_ofst,
		  g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
	iowrite32(this_loop_count, g_ioremap_desc_addr + DESC_LENGTH_REG);
	iowrite32(START_DMA_MASK, g_ioremap_desc_addr + DESC_CONTROL_REG);

	temp_dma_count -= this_loop_count;
	ram_ptr += this_loop_count;
	next_dma_io_buf_ofst += this_loop_count;
	if (next_dma_io_buf_ofst >= PAGE_SIZE)
		next_dma_io_buf_ofst = 0;

	while (temp_user_count > 0) {
		if (temp_dma_count > 0) {
			iowrite32(ram_ptr,
				  g_ioremap_desc_addr + DESC_READ_ADDRESS_REG);
			iowrite32(g_dma_handle_4k + next_dma_io_buf_ofst,
				  g_ioremap_desc_addr + DESC_WRITE_ADDRESS_REG);
			iowrite32(this_loop_count,
				  g_ioremap_desc_addr + DESC_LENGTH_REG);
			iowrite32(START_DMA_MASK,
				  g_ioremap_desc_addr + DESC_CONTROL_REG);

			temp_dma_count -= this_loop_count;
			ram_ptr += this_loop_count;
			next_dma_io_buf_ofst += this_loop_count;
			if (next_dma_io_buf_ofst >= PAGE_SIZE)
				next_dma_io_buf_ofst = 0;
		} else {
			while (get_dma_busy() != 0) {
				if (wait_event_interruptible(g_irq_wait_queue,
							     (get_dma_busy() ==
							      0))) {
					up(&dev->sem);
					pr_info
				   ("demo_dma_co_read wait interrupted exit\n");
					return -ERESTARTSYS;
				}
			}
		}

		while (get_dma_fill_level() > 0) {
			if (wait_event_interruptible(g_irq_wait_queue,
						     (get_dma_fill_level() ==
						      0))) {
				up(&dev->sem);
				pr_info
				   ("demo_dma_co_read wait interrupted exit\n");
				return -ERESTARTSYS;
			}
		}

		if (copy_to_user(user_buffer, g_coherent_ptr_4k +
				 next_user_io_buf_ofst, this_loop_count)) {
			up(&dev->sem);
			pr_info("demo_dma_co_read copy_to_user exit\n");
			return -EFAULT;
		}
		temp_user_count -= this_loop_count;
		user_buffer += this_loop_count;
		next_user_io_buf_ofst += this_loop_count;
		if (next_user_io_buf_ofst >= PAGE_SIZE)
			next_user_io_buf_ofst = 0;
	}

	*offset += count;

	up(&dev->sem);
	return count;
}

static const struct file_operations demo_dma_co_fops = {
	.owner = THIS_MODULE,
	.read = demo_dma_co_read,
	.write = demo_dma_co_write,
	.llseek = demo_dma_xx_llseek,
};

static struct miscdevice demo_dma_co_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "demo_dma_co",
	.fops = &demo_dma_co_fops,
};

/* platform driver */
irqreturn_t demo_driver_interrupt_handler(int irq, void *dev_id)
{
	spin_lock(&g_irq_lock);

	/* clear the IRQ state */
	iowrite32(ALTERA_MSGDMA_CSR_IRQ_SET_MASK,
		  g_ioremap_csr_addr + CSR_STATUS_REG);

	spin_unlock(&g_irq_lock);
	wake_up_interruptible(&g_irq_wait_queue);
	return IRQ_HANDLED;
}

static int platform_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	struct resource *demo_dma_csr_mem_region;
	struct resource *demo_dma_desc_mem_region;
	int irq;
	uint32_t dma_status;
	uint32_t dma_control;

	ret_val = -EBUSY;

	/* acquire the probe lock */
	if (down_interruptible(&g_dev_probe_sem))
		return -ERESTARTSYS;

	if (g_platform_probe_flag != 0)
		goto bad_exit_return;

	ret_val = -EINVAL;

	/* get our csr memory resource */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_return;
	}

	g_demo_dma_csr_addr = r->start;
	g_demo_dma_csr_size = resource_size(r);

	ret_val = -EBUSY;

	/* reserve our csr memory region */
	demo_dma_csr_mem_region = request_mem_region(g_demo_dma_csr_addr,
						     g_demo_dma_csr_size,
						     "demo_dma_csr_hw_region");
	if (demo_dma_csr_mem_region == NULL) {
		pr_err("request_mem_region failed: g_demo_dma_csr_addr\n");
		goto bad_exit_return;
	}
	/* ioremap our csr memory region */
	g_ioremap_csr_addr = ioremap(g_demo_dma_csr_addr, g_demo_dma_csr_size);
	if (g_ioremap_csr_addr == NULL) {
		pr_err("ioremap failed: g_demo_dma_csr_addr\n");
		goto bad_exit_release_mem_region_csr;
	}
	/* initialize the DMA controller */
	dma_status = ioread32(g_ioremap_csr_addr + CSR_STATUS_REG);
	if ((dma_status & (ALTERA_MSGDMA_CSR_BUSY_MASK |
			   ALTERA_MSGDMA_CSR_STOP_STATE_MASK |
			   ALTERA_MSGDMA_CSR_RESET_STATE_MASK |
			   ALTERA_MSGDMA_CSR_IRQ_SET_MASK)) != 0) {
		pr_err("initial dma status set unexpected: 0x%08X\n",
		       dma_status);
		goto bad_exit_release_mem_region_csr;
	}

	if ((dma_status & ALTERA_MSGDMA_CSR_DESCRIPTOR_BUFFER_EMPTY_MASK)
	    == 0) {
		pr_err("initial dma status cleared unexpected: 0x%08X\n",
		       dma_status);
		goto bad_exit_release_mem_region_csr;
	}

	dma_control = ioread32(g_ioremap_csr_addr + CSR_CONTROL_REG);
	if ((dma_control & (ALTERA_MSGDMA_CSR_STOP_MASK |
			    ALTERA_MSGDMA_CSR_RESET_MASK |
			    ALTERA_MSGDMA_CSR_STOP_ON_ERROR_MASK |
			    ALTERA_MSGDMA_CSR_STOP_ON_EARLY_TERMINATION_MASK |
			    ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK |
			    ALTERA_MSGDMA_CSR_STOP_DESCRIPTORS_MASK)) != 0) {
		pr_err("initial dma control set unexpected: 0x%08X\n",
		       (uint32_t)dma_control);
		goto bad_exit_release_mem_region_csr;
	}

	ret_val = -EINVAL;

	/* get our desc memory resource */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (r == NULL) {
		pr_err("IORESOURCE_MEM, 1 does not exist\n");
		goto bad_exit_iounmap_csr;
	}
	g_demo_dma_desc_addr = r->start;
	g_demo_dma_desc_size = resource_size(r);

	ret_val = -EBUSY;

	/* reserve our desc memory region */
	demo_dma_desc_mem_region = request_mem_region(g_demo_dma_desc_addr,
						      g_demo_dma_desc_size,
						      "demo_dma_desc_hw_region");
	if (demo_dma_desc_mem_region == NULL) {
		pr_err("request_mem_region failed: g_demo_dma_desc_addr\n");
		goto bad_exit_iounmap_csr;
	}
	/* ioremap our desc memory region */
	g_ioremap_desc_addr =
	    ioremap(g_demo_dma_desc_addr, g_demo_dma_desc_size);
	if (g_ioremap_desc_addr == NULL) {
		pr_err("ioremap failed: g_demo_dma_desc_addr\n");
		goto bad_exit_release_mem_region_desc;
	}
	/* get our interrupt resource */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("irq not available\n");
		goto bad_exit_iounmap_desc;
	}
	g_demo_dma_irq = irq;

	/* allocate some memory buffers */
	g_kmalloc_ptr_4k = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (g_kmalloc_ptr_4k == NULL) {
		pr_err("kmalloc failed\n");
		goto bad_exit_iounmap_desc;
	}

	the_demo_dma_xx.pdev_dev = &pdev->dev;
	g_coherent_ptr_4k =
	    dma_alloc_coherent(the_demo_dma_xx.pdev_dev, PAGE_SIZE,
			       &g_dma_handle_4k, GFP_KERNEL);
	if (g_coherent_ptr_4k == NULL) {
		pr_err("dma_alloc_coherent failed 4KB\n");
		goto bad_exit_kfree;
	}

	g_coherent_ptr_1m = dma_alloc_coherent(the_demo_dma_xx.pdev_dev,
					       DMA_DEVICE_BUFFER_SIZE,
					       &g_dma_handle_1m, GFP_KERNEL);
	if (g_coherent_ptr_1m == NULL) {
		pr_err("dma_alloc_coherent failed 1MB\n");
		goto bad_exit_dma_free_coherent_4k;
	}
	/* register our interrupt handler */
	init_waitqueue_head(&g_irq_wait_queue);
	spin_lock_init(&g_irq_lock);
	g_irq_count = 0;
	g_max_irq_delay = 0;
	g_min_irq_delay = 0xFFFFFFFF;

	ret_val = request_irq(g_demo_dma_irq,
			      demo_driver_interrupt_handler,
			      0,
			      the_platform_driver.driver.name,
			      &the_platform_driver);

	if (ret_val) {
		pr_err("request_irq failed");
		goto bad_exit_dma_free_coherent_1m;
	}
	/* enable the DMA global IRQ mask */
	iowrite32(ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK,
		  g_ioremap_csr_addr + CSR_CONTROL_REG);

	/* register misc device demo_dma_co */
	sema_init(&the_demo_dma_xx.sem, 1);
	ret_val = misc_register(&demo_dma_co_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_dma_co\"...");
		goto bad_exit_freeirq;
	}
	/* register misc device demo_dma_st */
	ret_val = misc_register(&demo_dma_st_device);
	if (ret_val != 0) {
		pr_warn("Could not register device \"demo_dma_st\"...");
		goto bad_exit_deregister_demo_dma_co;
	}

	g_platform_probe_flag = 1;
	up(&g_dev_probe_sem);
	return 0;

bad_exit_deregister_demo_dma_co:
	misc_deregister(&demo_dma_co_device);
bad_exit_freeirq:
	free_irq(g_demo_dma_irq, &the_platform_driver);
bad_exit_dma_free_coherent_1m:
	dma_free_coherent(the_demo_dma_xx.pdev_dev, DMA_DEVICE_BUFFER_SIZE,
			  g_coherent_ptr_1m, g_dma_handle_1m);
bad_exit_dma_free_coherent_4k:
	dma_free_coherent(the_demo_dma_xx.pdev_dev, PAGE_SIZE,
			  g_coherent_ptr_4k, g_dma_handle_4k);
bad_exit_kfree:
	kfree(g_kmalloc_ptr_4k);
bad_exit_iounmap_desc:
	iounmap(g_ioremap_desc_addr);
bad_exit_release_mem_region_desc:
	release_mem_region(g_demo_dma_desc_addr, g_demo_dma_desc_size);
bad_exit_iounmap_csr:
	iounmap(g_ioremap_csr_addr);
bad_exit_release_mem_region_csr:
	release_mem_region(g_demo_dma_csr_addr, g_demo_dma_csr_size);
bad_exit_return:
	up(&g_dev_probe_sem);
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	misc_deregister(&demo_dma_st_device);
	misc_deregister(&demo_dma_co_device);

	/* disable the DMA global IRQ mask */
	iowrite32(0, g_ioremap_csr_addr + CSR_CONTROL_REG);

	free_irq(g_demo_dma_irq, &the_platform_driver);
	dma_free_coherent(the_demo_dma_xx.pdev_dev, DMA_DEVICE_BUFFER_SIZE,
			  g_coherent_ptr_1m, g_dma_handle_1m);
	dma_free_coherent(the_demo_dma_xx.pdev_dev, PAGE_SIZE,
			  g_coherent_ptr_4k, g_dma_handle_4k);
	kfree(g_kmalloc_ptr_4k);
	iounmap(g_ioremap_desc_addr);
	release_mem_region(g_demo_dma_desc_addr, g_demo_dma_desc_size);
	iounmap(g_ioremap_csr_addr);
	release_mem_region(g_demo_dma_csr_addr, g_demo_dma_csr_size);

	if (down_interruptible(&g_dev_probe_sem))
		return -ERESTARTSYS;

	g_platform_probe_flag = 0;
	up(&g_dev_probe_sem);

	return 0;
}

static struct of_device_id demo_driver_dt_ids[] = {
	{
	 .compatible = "demo,memcpy_msgdma"},
	{ /* end of table */ }
};

MODULE_DEVICE_TABLE(of, demo_driver_dt_ids);

static struct platform_driver the_platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		   .name = "demo_driver_9",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
};

static int demo_init(void)
{
	int ret_val;

	sema_init(&g_dev_probe_sem, 1);

	ret_val = platform_driver_register(&the_platform_driver);
	if (ret_val != 0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		return ret_val;
	}

	return 0;
}

static void demo_exit(void)
{
	platform_driver_unregister(&the_platform_driver);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_DESCRIPTION("Demonstration Module 9 - introduce dma");
MODULE_VERSION("1.0");
