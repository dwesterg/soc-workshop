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
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "demo_module.h"

/* defines */
#define NAME_BUF_SIZE	(32)

/* globals */
static struct list_head g_dev_list;
static struct semaphore g_dev_list_sem;
static int g_dev_index;

/* device structure */
struct demo_fifo_dev {
	char name[NAME_BUF_SIZE];
	char in_region_name[NAME_BUF_SIZE];
	char out_region_name[NAME_BUF_SIZE];
	char in_csr_region_name[NAME_BUF_SIZE];
	struct list_head dev_list;
	struct semaphore dev_sem;
	wait_queue_head_t wait_queue;
	struct resource *in_res;
	struct resource *out_res;
	struct resource *in_csr_res;
	void __iomem *ioremap_in_addr;
	void __iomem *ioremap_out_addr;
	void __iomem *ioremap_in_csr_addr;
	uint32_t open_for_read;
	uint32_t open_for_write;
	struct miscdevice miscdev;
};

/* misc device - demo_fifo */
static ssize_t demo_fifo_write(struct file *fp,
			       const char __user *user_buffer, size_t count,
			       loff_t *offset)
{
	struct demo_fifo_dev *the_demo_fifo_dev = fp->private_data;
	uint32_t temp_data;
	int this_count = 0;
	uint32_t fifo_level;

	if (down_interruptible(&the_demo_fifo_dev->dev_sem)) {
		pr_info("demo_fifo_write sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (count & (4 - 1)) {
		up(&the_demo_fifo_dev->dev_sem);
		pr_info("demo_fifo_write count not multiple of 4 exit\n");
		return -EINVAL;
	}

	while (ioread32(the_demo_fifo_dev->ioremap_in_csr_addr + FIFO_LEVEL_REG)
	       >= FIFO_MAX_FILL_LEVEL) {
		up(&the_demo_fifo_dev->dev_sem);
		if (wait_event_interruptible(the_demo_fifo_dev->wait_queue,
					     (ioread32
					      (the_demo_fifo_dev->
					       ioremap_in_csr_addr
					       + FIFO_LEVEL_REG) <
					      FIFO_MAX_FILL_LEVEL))) {
			pr_info("demo_fifo_write wait interrupted exit\n");
			return -ERESTARTSYS;
		}
		if (down_interruptible(&the_demo_fifo_dev->dev_sem)) {
			pr_info("demo_fifo_write sem interrupted exit\n");
			return -ERESTARTSYS;
		}
	}

	fifo_level = ioread32(the_demo_fifo_dev->ioremap_in_csr_addr +
			      FIFO_LEVEL_REG);

	while (fifo_level < FIFO_MAX_FILL_LEVEL) {
		if (copy_from_user(&temp_data, user_buffer, 4)) {
			up(&the_demo_fifo_dev->dev_sem);
			pr_info("demo_fifo_write copy_to_user exit\n");
			return -EFAULT;
		}
		iowrite32(temp_data, the_demo_fifo_dev->ioremap_in_addr +
			  FIFO_DATA_REG);

		user_buffer += 4;
		fifo_level++;
		this_count += 4;
		count -= 4;
		if (count == 0)
			break;
	}

	up(&the_demo_fifo_dev->dev_sem);
	wake_up_interruptible(&the_demo_fifo_dev->wait_queue);

	return this_count;
}

static ssize_t demo_fifo_read(struct file *fp, char __user *user_buffer,
			      size_t count, loff_t *offset)
{
	struct demo_fifo_dev *the_demo_fifo_dev = fp->private_data;
	uint32_t temp_data;
	int this_count = 0;
	uint32_t fifo_level;

	if (down_interruptible(&the_demo_fifo_dev->dev_sem)) {
		pr_info("demo_fifo_read sem interrupted exit\n");
		return -ERESTARTSYS;
	}

	if (count & (4 - 1)) {
		up(&the_demo_fifo_dev->dev_sem);
		pr_info("demo_fifo_read count not multiple of 4 exit\n");
		return -EINVAL;
	}

	while (ioread32(the_demo_fifo_dev->ioremap_in_csr_addr + FIFO_LEVEL_REG)
	       == 0) {
		up(&the_demo_fifo_dev->dev_sem);
		if (wait_event_interruptible(the_demo_fifo_dev->wait_queue,
					     (ioread32
					      (the_demo_fifo_dev->
					       ioremap_in_csr_addr
					       + FIFO_LEVEL_REG) != 0))) {
			pr_info("demo_fifo_read wait interrupted exit\n");
			return -ERESTARTSYS;
		}
		if (down_interruptible(&the_demo_fifo_dev->dev_sem)) {
			pr_info("demo_fifo_read sem interrupted exit\n");
			return -ERESTARTSYS;
		}
	}

	fifo_level = ioread32(the_demo_fifo_dev->ioremap_in_csr_addr +
			      FIFO_LEVEL_REG);

	while (fifo_level > 0) {
		temp_data = ioread32(the_demo_fifo_dev->ioremap_out_addr +
				     FIFO_DATA_REG);
		if (copy_to_user(user_buffer, &temp_data, 4)) {
			up(&the_demo_fifo_dev->dev_sem);
			pr_info("demo_fifo_read copy_to_user exit\n");
			return -EFAULT;
		}
		user_buffer += 4;
		fifo_level--;
		this_count += 4;
		count -= 4;
		if (count == 0)
			break;
	}

	up(&the_demo_fifo_dev->dev_sem);
	wake_up_interruptible(&the_demo_fifo_dev->wait_queue);

	return this_count;
}

static int demo_fifo_open(struct inode *ip, struct file *fp)
{
	struct demo_fifo_dev *the_demo_fifo_dev = NULL;
	uint32_t this_minor;
	uint32_t access_mode;
	struct list_head *next_list_entry;
	int found_it = 0;

	if (down_interruptible(&g_dev_list_sem))
		return -ERESTARTSYS;

	this_minor = iminor(ip);

	list_for_each(next_list_entry, &g_dev_list) {
		the_demo_fifo_dev = list_entry(next_list_entry,
					       struct demo_fifo_dev, dev_list);
		if (the_demo_fifo_dev->miscdev.minor == this_minor) {
			found_it = 1;
			break;
		}
	}

	up(&g_dev_list_sem);

	if (found_it == 0)
		return -ENXIO;

	access_mode = fp->f_flags & O_ACCMODE;
	switch (access_mode) {
	case (O_RDONLY):
		if (the_demo_fifo_dev->open_for_read != 0)
			return -EBUSY;
		the_demo_fifo_dev->open_for_read = 1;
		break;
	case (O_WRONLY):
		if (the_demo_fifo_dev->open_for_write != 0)
			return -EBUSY;
		the_demo_fifo_dev->open_for_write = 1;
		break;
	case (O_RDWR):
		if (the_demo_fifo_dev->open_for_read != 0)
			return -EBUSY;
		if (the_demo_fifo_dev->open_for_write != 0)
			return -EBUSY;
		the_demo_fifo_dev->open_for_read = 1;
		the_demo_fifo_dev->open_for_write = 1;
		break;
	default:
		return -EINVAL;
	}

	fp->private_data = the_demo_fifo_dev;

	return 0;
}

static int demo_fifo_release(struct inode *ip, struct file *fp)
{
	struct demo_fifo_dev *the_demo_fifo_dev = fp->private_data;
	uint32_t access_mode;

	access_mode = fp->f_flags & O_ACCMODE;
	switch (access_mode) {
	case (O_RDONLY):
		the_demo_fifo_dev->open_for_read = 0;
		break;
	case (O_WRONLY):
		the_demo_fifo_dev->open_for_write = 0;
		break;
	case (O_RDWR):
		the_demo_fifo_dev->open_for_read = 0;
		the_demo_fifo_dev->open_for_write = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations demo_fifo_fops = {
	.owner = THIS_MODULE,
	.open = demo_fifo_open,
	.release = demo_fifo_release,
	.read = demo_fifo_read,
	.write = demo_fifo_write,
};

/* platform driver */
static int platform_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r0 = NULL;
	struct resource *r1 = NULL;
	struct resource *r2 = NULL;
	struct resource *temp_res = NULL;
	struct demo_fifo_dev *the_demo_fifo_dev;
	uint32_t fifo_level;

	if (down_interruptible(&g_dev_list_sem))
		return -ERESTARTSYS;

	ret_val = -ENOMEM;

	/* allocate a demo_fifo_dev structure */
	the_demo_fifo_dev = kzalloc(sizeof(struct demo_fifo_dev), GFP_KERNEL);
	if (the_demo_fifo_dev == NULL) {
		pr_err("kzalloc failed\n");
		goto bad_exit_return;
	}
	/* initialize the demo_fifo_dev structure */
	scnprintf(the_demo_fifo_dev->name, NAME_BUF_SIZE, "demo_fifo_%d",
		  g_dev_index);

	INIT_LIST_HEAD(&the_demo_fifo_dev->dev_list);
	sema_init(&the_demo_fifo_dev->dev_sem, 1);
	init_waitqueue_head(&the_demo_fifo_dev->wait_queue);

	the_demo_fifo_dev->in_res = NULL;
	the_demo_fifo_dev->out_res = NULL;
	the_demo_fifo_dev->in_csr_res = NULL;

	the_demo_fifo_dev->ioremap_in_addr = NULL;
	the_demo_fifo_dev->ioremap_out_addr = NULL;
	the_demo_fifo_dev->ioremap_in_csr_addr = NULL;

	the_demo_fifo_dev->open_for_read = 0;
	the_demo_fifo_dev->open_for_write = 0;

	the_demo_fifo_dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	the_demo_fifo_dev->miscdev.name = the_demo_fifo_dev->name;
	the_demo_fifo_dev->miscdev.fops = &demo_fifo_fops, ret_val = -EINVAL;

	/* get our three expected memory resources */
	r0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r0 == NULL) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	r1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (r1 == NULL) {
		pr_err("IORESOURCE_MEM, 1 does not exist\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	r2 = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (r2 == NULL) {
		pr_err("IORESOURCE_MEM, 2 does not exist\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	/* associate the resources for in, out and in_csr */

	if (!strcmp(r0->name, "in"))
		the_demo_fifo_dev->in_res = r0;
	else if (!strcmp(r1->name, "in"))
		the_demo_fifo_dev->in_res = r1;
	else if (!strcmp(r2->name, "in"))
		the_demo_fifo_dev->in_res = r2;

	if (!strcmp(r0->name, "out"))
		the_demo_fifo_dev->out_res = r0;
	else if (!strcmp(r1->name, "out"))
		the_demo_fifo_dev->out_res = r1;
	else if (!strcmp(r2->name, "out"))
		the_demo_fifo_dev->out_res = r2;

	if (!strcmp(r0->name, "in_csr"))
		the_demo_fifo_dev->in_csr_res = r0;
	else if (!strcmp(r1->name, "in_csr"))
		the_demo_fifo_dev->in_csr_res = r1;
	else if (!strcmp(r2->name, "in_csr"))
		the_demo_fifo_dev->in_csr_res = r2;

	/* verify that we found all three resources */
	if (the_demo_fifo_dev->in_res == NULL) {
		pr_err("no resource found for \"in\"\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	if (the_demo_fifo_dev->out_res == NULL) {
		pr_err("no resource found for \"out\"\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	if (the_demo_fifo_dev->in_csr_res == NULL) {
		pr_err("no resource found for \"in_csr\"\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	ret_val = -EBUSY;

	/* reserve our memory regions */
	temp_res = request_mem_region(the_demo_fifo_dev->in_res->start,
				      resource_size(the_demo_fifo_dev->in_res),
				      strncat(strncpy
					      (the_demo_fifo_dev->
					       in_region_name,
					       pdev->name, NAME_BUF_SIZE),
					      ".in", NAME_BUF_SIZE));
	if (temp_res == NULL) {
		pr_err("request_mem_region failed: \"in\"\n");
		goto bad_exit_kfree_the_demo_fifo_dev;
	}

	temp_res = request_mem_region(the_demo_fifo_dev->out_res->start,
				      resource_size(the_demo_fifo_dev->out_res),
				      strncat(strncpy
					      (the_demo_fifo_dev->
					       out_region_name,
					       pdev->name, NAME_BUF_SIZE),
					      ".out", NAME_BUF_SIZE));
	if (temp_res == NULL) {
		pr_err("request_mem_region failed: \"out\"\n");
		goto bad_exit_release_mem_region_in;
	}

	temp_res = request_mem_region(the_demo_fifo_dev->in_csr_res->start,
				      resource_size
				      (the_demo_fifo_dev->in_csr_res),
				      strncat(strncpy
					      (the_demo_fifo_dev->
					       in_csr_region_name,
					       pdev->name, NAME_BUF_SIZE),
					      ".in_csr", NAME_BUF_SIZE));
	if (temp_res == NULL) {
		pr_err("request_mem_region failed: \"in_csr\"\n");
		goto bad_exit_release_mem_region_out;
	}

	/* ioremap our memory regions */
	the_demo_fifo_dev->ioremap_in_addr =
	    ioremap(the_demo_fifo_dev->in_res->start,
		    resource_size(the_demo_fifo_dev->in_res));
	if (the_demo_fifo_dev->ioremap_in_addr == NULL) {
		pr_err("ioremap failed: ioremap_in_addr\n");
		goto bad_exit_release_mem_region_in_csr;
	}

	the_demo_fifo_dev->ioremap_out_addr =
	    ioremap(the_demo_fifo_dev->out_res->start,
		    resource_size(the_demo_fifo_dev->out_res));
	if (the_demo_fifo_dev->ioremap_out_addr == NULL) {
		pr_err("ioremap failed: ioremap_out_addr\n");
		goto bad_exit_iounmap_in;
	}

	the_demo_fifo_dev->ioremap_in_csr_addr =
	    ioremap(the_demo_fifo_dev->in_csr_res->start,
		    resource_size(the_demo_fifo_dev->in_csr_res));
	if (the_demo_fifo_dev->ioremap_in_csr_addr == NULL) {
		pr_err("ioremap failed: ioremap_in_csr_addr\n");
		goto bad_exit_iounmap_out;
	}

	ret_val = -EIO;

	/* initialize the FIFO hardware */
	fifo_level = ioread32(the_demo_fifo_dev->ioremap_in_csr_addr +
			      FIFO_LEVEL_REG);
	while (fifo_level > 0) {
		ioread32(the_demo_fifo_dev->ioremap_out_addr + FIFO_DATA_REG);
		fifo_level--;
	}

	fifo_level = ioread32(the_demo_fifo_dev->ioremap_in_csr_addr +
			      FIFO_LEVEL_REG);

	if (fifo_level != 0) {
		pr_err("fifo initialization failed");
		goto bad_exit_iounmap_in_csr;
	}

	ret_val = -EINVAL;

	/* register misc device demo_fifo */
	ret_val = misc_register(&the_demo_fifo_dev->miscdev);
	if (ret_val != 0) {
		pr_warn("Could not register device \"%s\"...",
			the_demo_fifo_dev->name);
		goto bad_exit_iounmap_in_csr;
	}

	/* clean up and exit */
	list_add(&the_demo_fifo_dev->dev_list, &g_dev_list);
	g_dev_index++;
	platform_set_drvdata(pdev, the_demo_fifo_dev);
	up(&g_dev_list_sem);
	return 0;

bad_exit_iounmap_in_csr:
	iounmap(the_demo_fifo_dev->ioremap_in_csr_addr);
bad_exit_iounmap_out:
	iounmap(the_demo_fifo_dev->ioremap_out_addr);
bad_exit_iounmap_in:
	iounmap(the_demo_fifo_dev->ioremap_in_addr);
bad_exit_release_mem_region_in_csr:
	release_mem_region(the_demo_fifo_dev->in_csr_res->start,
			   resource_size(the_demo_fifo_dev->in_csr_res));
bad_exit_release_mem_region_out:
	release_mem_region(the_demo_fifo_dev->out_res->start,
			   resource_size(the_demo_fifo_dev->out_res));
bad_exit_release_mem_region_in:
	release_mem_region(the_demo_fifo_dev->in_res->start,
			   resource_size(the_demo_fifo_dev->in_res));
bad_exit_kfree_the_demo_fifo_dev:
	kfree(the_demo_fifo_dev);
bad_exit_return:
	up(&g_dev_list_sem);
	return ret_val;
}

static int platform_remove(struct platform_device *pdev)
{
	struct demo_fifo_dev *the_demo_fifo_dev;

	if (down_interruptible(&g_dev_list_sem))
		return -ERESTARTSYS;

	the_demo_fifo_dev = platform_get_drvdata(pdev);

	list_del_init(&the_demo_fifo_dev->dev_list);

	misc_deregister(&the_demo_fifo_dev->miscdev);
	iounmap(the_demo_fifo_dev->ioremap_in_csr_addr);
	iounmap(the_demo_fifo_dev->ioremap_out_addr);
	iounmap(the_demo_fifo_dev->ioremap_in_addr);
	release_mem_region(the_demo_fifo_dev->in_csr_res->start,
			   resource_size(the_demo_fifo_dev->in_csr_res));
	release_mem_region(the_demo_fifo_dev->out_res->start,
			   resource_size(the_demo_fifo_dev->out_res));
	release_mem_region(the_demo_fifo_dev->in_res->start,
			   resource_size(the_demo_fifo_dev->in_res));
	kfree(the_demo_fifo_dev);

	up(&g_dev_list_sem);
	return 0;
}

static struct of_device_id demo_driver_dt_ids[] = {
	{
	 .compatible = "ALTR,fifo-1.0"},
	{ /* end of table */ }
};

MODULE_DEVICE_TABLE(of, demo_driver_dt_ids);

static struct platform_driver the_platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		   .name = "demo_driver_10",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
};

static int demo_init(void)
{
	int ret_val;

	INIT_LIST_HEAD(&g_dev_list);
	sema_init(&g_dev_list_sem, 1);
	g_dev_index = 0;

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
MODULE_DESCRIPTION("Demonstration Module 10 - multiple device instances");
MODULE_VERSION("1.0");
