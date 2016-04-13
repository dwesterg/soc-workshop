/*
 *  Copyright (C) 2014 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

#include "validator_module.h"

#if (UPPER_HALF_SIZE != 512)
#error UPPER_HALF_SIZE not equal to 512, this code assumes that relationship
#endif

static int g_validator_base_addr;
static void *g_ioremap_addr;
static struct semaphore g_driver_mutex;
static struct semaphore g_sign_read_mutex;
static struct semaphore g_sign_write_mutex;
static struct semaphore g_auth_read_mutex;
static struct semaphore g_auth_write_mutex;
static uint8_t g_io_buffer[1024];

//
// misc device - validator_entropy_dev
//
static ssize_t validator_entropy_dev_read (struct file *fp, 
		char __user *user_buffer, size_t count, loff_t *offset) {
	
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;

	if(count != 1024)
		return -EINVAL;

	if(*offset > 0)
		return -EINVAL;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG)
		return -EIO;

	iowrite32(DUMP_ENTROPY_COUNTERS_LO_REQ,
			g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec))
			return -EIO;
		schedule();
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	memcpy_fromio(g_io_buffer, g_ioremap_addr + UPPER_HALF_OFFSET, 
			UPPER_HALF_SIZE);

	iowrite32(DUMP_ENTROPY_COUNTERS_HI_REQ,
			g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec))
			return -EIO;
		schedule();
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	memcpy_fromio(g_io_buffer + UPPER_HALF_SIZE, 
			g_ioremap_addr + UPPER_HALF_OFFSET, 
			UPPER_HALF_SIZE);

	if(copy_to_user(user_buffer, g_io_buffer, 1024))
		return -EFAULT;

	//*offset += count;

	return count;
}

static int validator_entropy_dev_open (struct inode *ip, struct file *fp) {

	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;
	
	return 0;
}

static int validator_entropy_dev_release (struct inode *ip, struct file *fp) {

	up(&g_driver_mutex);
	return 0;
}

static const struct file_operations validator_entropy_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= validator_entropy_dev_open,
	.release	= validator_entropy_dev_release,
	.read		= validator_entropy_dev_read,
};

static struct miscdevice validator_entropy_dev_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "validator_entropy",
	.fops	= &validator_entropy_dev_fops,
};

//
// misc device - validator_rand_dev
//
static ssize_t validator_rand_dev_read (struct file *fp,
		char __user *user_buffer, size_t count, loff_t *offset) {
	
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(count > UPPER_HALF_SIZE)
		count = UPPER_HALF_SIZE;

	if(*offset > 0)
		return -EINVAL;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG)
		return -EIO;

	iowrite32(DUMP_RANDOM_NUMBERS_REQ, g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 6000;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec))
			return -EIO;
		schedule();
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	memcpy_fromio(g_io_buffer, g_ioremap_addr + UPPER_HALF_OFFSET, 
			UPPER_HALF_SIZE);

	if(copy_to_user(user_buffer, g_io_buffer, count))
		return -EFAULT;

	//*offset += count;

	return count;
}

static int validator_rand_dev_open (struct inode *ip, struct file *fp) {

	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;
	
	return 0;
}

static int validator_rand_dev_release (struct inode *ip, struct file *fp) {

	up(&g_driver_mutex);
	return 0;
}

static const struct file_operations validator_rand_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= validator_rand_dev_open,
	.release	= validator_rand_dev_release,
	.read		= validator_rand_dev_read,
};

static struct miscdevice validator_rand_dev_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "validator_rand",
	.fops	= &validator_rand_dev_fops,
};

//
// misc device - validator_auth_dev
//
static ssize_t validator_auth_dev_read (struct file *fp, 
		char __user *user_buffer, size_t count, loff_t *offset) {
	
	if(count != 4)
		return -EINVAL;

	if(*offset > 0)
		return -EINVAL;

	if(down_interruptible(&g_auth_read_mutex))
		return -ERESTARTSYS;

	if(copy_to_user(user_buffer, g_io_buffer, count))
		return -EFAULT;

	//*offset += count;

	up(&g_auth_write_mutex);

	return count;
}

static ssize_t validator_auth_dev_write (struct file *fp, 
		const char __user *user_buffer, size_t count, loff_t *offset) {
			
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(count != 96)
		return -EINVAL;

	if(*offset > 0)
		return -EINVAL;

	if(down_interruptible(&g_auth_write_mutex))
		return -ERESTARTSYS;
	
	if(copy_from_user(g_io_buffer, user_buffer, count))
		return -EFAULT;

	//*offset += count;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG)
		return -EIO;

	memcpy_toio(g_ioremap_addr + MESSAGE_OUT_OFFSET, g_io_buffer, count);
	iowrite32(AUTHENTICATE_MESSAGE_REQ, g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec))
			return -EIO;
		schedule();
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	*(uint32_t *)(g_io_buffer) = ioread32(g_ioremap_addr + RESULT_OFFSET);
	
	up(&g_auth_read_mutex);

	return count;
}

static int validator_auth_dev_open (struct inode *ip, struct file *fp) {

	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;
	
	sema_init(&g_auth_read_mutex, 0);
	sema_init(&g_auth_write_mutex, 1);

	return 0;
}

static int validator_auth_dev_release (struct inode *ip, struct file *fp) {

	up(&g_driver_mutex);
	return 0;
}

static const struct file_operations validator_auth_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= validator_auth_dev_open,
	.release	= validator_auth_dev_release,
	.read		= validator_auth_dev_read,
	.write		= validator_auth_dev_write,
};

static struct miscdevice validator_auth_dev_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "validator_auth",
	.fops	= &validator_auth_dev_fops,
};

//
// misc device - validator_sign_dev
//
static ssize_t validator_sign_dev_read (struct file *fp, 
		char __user *user_buffer, size_t count, loff_t *offset) {
	
	if(count != 96)
		return -EINVAL;

	if(*offset > 0)
		return -EINVAL;

	if(down_interruptible(&g_sign_read_mutex))
		return -ERESTARTSYS;

	if(copy_to_user(user_buffer, g_io_buffer, count))
		return -EFAULT;

	//*offset += count;

	up(&g_sign_write_mutex);

	return count;
}

static ssize_t validator_sign_dev_write (struct file *fp, 
		const char __user *user_buffer, size_t count, loff_t *offset) {
			
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(count != 32)
		return -EINVAL;

	if(*offset > 0)
		return -EINVAL;

	if(down_interruptible(&g_sign_write_mutex))
		return -ERESTARTSYS;
	
	if(copy_from_user(g_io_buffer, user_buffer, count))
		return -EFAULT;

	//*offset += count;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG)
		return -EIO;

	memcpy_toio(g_ioremap_addr + MESSAGE_IN_OFFSET, g_io_buffer, count);
	iowrite32(SIGN_MESSAGE_REQ, g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec))
			return -EIO;
		schedule();
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	memcpy_fromio(g_io_buffer, g_ioremap_addr + MESSAGE_OUT_OFFSET, 96);

	up(&g_sign_read_mutex);

	return count;
}

static int validator_sign_dev_open (struct inode *ip, struct file *fp) {

	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;
	
	sema_init(&g_sign_read_mutex, 0);
	sema_init(&g_sign_write_mutex, 1);

	return 0;
}

static int validator_sign_dev_release (struct inode *ip, struct file *fp) {

	up(&g_driver_mutex);
	return 0;
}

static const struct file_operations validator_sign_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= validator_sign_dev_open,
	.release	= validator_sign_dev_release,
	.read		= validator_sign_dev_read,
	.write		= validator_sign_dev_write,
};

static struct miscdevice validator_sign_dev_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "validator_sign",
	.fops	= &validator_sign_dev_fops,
};

//
// misc device - validator_raw_dev
//
static int validator_raw_dev_mmap (struct file *fp, 
		struct vm_area_struct *vma) {

	if (vma->vm_end - vma->vm_start != PAGE_SIZE)
		return -EINVAL;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	vma->vm_pgoff = g_validator_base_addr >> PAGE_SHIFT;

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, PAGE_SIZE, 
			vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

static int validator_raw_dev_open (struct inode *ip, struct file *fp) {

	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;
		
	return 0;
}

static int validator_raw_dev_release (struct inode *ip, struct file *fp) {

	up(&g_driver_mutex);
	return 0;
}

static const struct file_operations validator_raw_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= validator_raw_dev_open,
	.release	= validator_raw_dev_release,
	.mmap		= validator_raw_dev_mmap,
};

static struct miscdevice validator_raw_dev_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "validator_raw",
	.fops	= &validator_raw_dev_fops,
};

//
// Platform Driver
// 
static ssize_t entropy_state_show(struct device_driver *driver, char *buf) {

	uint32_t result;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG) {
		up(&g_driver_mutex);
		return -EIO;
	}

	iowrite32(GET_ENTROPY_COUNTERS_STATE_REQ,
			g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec)) {
			up(&g_driver_mutex);
			return -EIO;
		}
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	result = ioread32(g_ioremap_addr + RESULT_OFFSET);

	up(&g_driver_mutex);
	return scnprintf(buf, PAGE_SIZE, "%s\n", 
			(result != 0) ? "ENABLED" : "DISABLED");
}

static ssize_t entropy_state_store(struct device_driver *driver,
		const char *buf, size_t count) {
		
	int result;
	ulong value;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG) {
		up(&g_driver_mutex);
		return -EIO;
	}

	result = kstrtoul(buf, 0, &value);
	if(result != 0) {
		if(strncasecmp(buf, "start", strlen("start")) == 0) {
			value = 1;
		} else {
			if(strncasecmp(buf, "stop", strlen("stop")) == 0) {
				value = 0;
			} else {
				up(&g_driver_mutex);
				return -EINVAL;
			}
		}
	}
	if(value != 0) {
		if(value != 1) {
			up(&g_driver_mutex);
			return -EINVAL;
		} else {
			iowrite32(START_ENTROPY_COUNTERS_REQ,
					g_ioremap_addr + ARM_REQ_OFFSET);
		}
	} else {
		iowrite32(STOP_ENTROPY_COUNTERS_REQ,
			g_ioremap_addr + ARM_REQ_OFFSET);
	}
	
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec)) {
			up(&g_driver_mutex);
			return -EIO;
		}
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	up(&g_driver_mutex);
	return(count);
}

DRIVER_ATTR(entropy_state, (S_IRUGO|S_IWUSR|S_IWGRP), entropy_state_show, 
		entropy_state_store);

static ssize_t transaction_counter_show(struct device_driver *driver,
		 char *buf) {

	uint32_t result;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG) {
		up(&g_driver_mutex);
		return -EIO;
	}

	iowrite32(DUMP_ENTROPY_COUNTERS_LO_REQ,
			g_ioremap_addr + ARM_REQ_OFFSET);
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec)) {
			up(&g_driver_mutex);
			return -EIO;
		}
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	result = ioread32(g_ioremap_addr + RESULT_OFFSET);

	up(&g_driver_mutex);
	return scnprintf(buf, PAGE_SIZE, "%u\n", result);
}

static ssize_t transaction_counter_store(struct device_driver *driver,
		const char *buf, size_t count) {

	int result;
	ulong value;
	struct timeval start_time;
	struct timeval end_time;
	struct timeval timeout_time;
	
	if(down_trylock(&g_driver_mutex) != 0)
		return -EAGAIN;

	if(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG) {
		up(&g_driver_mutex);
		return -EIO;
	}

	result = kstrtoul(buf, 0, &value);
	if(result != 0) {
		if(strncasecmp(buf, "reset", strlen("reset")) == 0) {
			value = 1;
		} else {
			up(&g_driver_mutex);
			return -EINVAL;
		}
	}
	if(value != 0) {
		if(value != 1) {
			up(&g_driver_mutex);
			return -EINVAL;
		} else {
			iowrite32(RESET_ENTROPY_COUNTERS_REQ,
					g_ioremap_addr + ARM_REQ_OFFSET);
		}
	} else {
		up(&g_driver_mutex);
		return(count);
	}
	
	mb();
	iowrite8(NIOS_OWNS_FLAG, g_ioremap_addr + HS_FLAGS_OFFSET);

	do_gettimeofday(&start_time);
	timeout_time.tv_sec = start_time.tv_sec;
	timeout_time.tv_usec = start_time.tv_usec + 500;
	if(timeout_time.tv_usec >= 1000000) {
		timeout_time.tv_sec++;
		timeout_time.tv_usec -= 1000000;
	}

	do {
		do_gettimeofday(&end_time);
		if((end_time.tv_sec >= timeout_time.tv_sec) && 
			(end_time.tv_usec >= timeout_time.tv_usec)) {
			up(&g_driver_mutex);
			return -EIO;
		}
	} while(ioread8(g_ioremap_addr + HS_FLAGS_OFFSET) != ARM_OWNS_FLAG);
	
	up(&g_driver_mutex);
	return(count);
}

DRIVER_ATTR(transaction_counter, (S_IRUGO|S_IWUSR|S_IWGRP), transaction_counter_show, 
		transaction_counter_store);

static struct driver_attribute *driver_attribute_array[2] = {
	&driver_attr_entropy_state,
	&driver_attr_transaction_counter,
};

static struct of_device_id validator_dt_ids[] = {
	{
		.compatible = "demo,validator-1.0"
	},
	{ /* end of table */ }
};
MODULE_DEVICE_TABLE(of, validator_dt_ids);

static int platform_probe(struct platform_device *pdev);
static int platform_remove(struct platform_device *pdev);

static struct platform_driver the_platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		.name = "validator_driver",
		.owner = THIS_MODULE,
		.of_match_table = validator_dt_ids,
	}
};

static int platform_probe(struct platform_device *pdev) {

	int ret_val;
	struct resource *validator_mem_region;
	struct resource *r;

	ret_val = -EINVAL;
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(r != NULL) {
		if(r->start & ~PAGE_MASK) {
			pr_err("validator base is not aligned to PAGE_SIZE\n");
			goto bad_exit_return;
		}
		if(((r->end - r->start) + 1) > PAGE_SIZE) {
			pr_err("validator span is larger than PAGE_SIZE\n");
			goto bad_exit_return;
		}
	} else {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_return;
	}

	g_validator_base_addr = r->start;
	
	sema_init(&g_driver_mutex, 0);
	
	ret_val = -EBUSY;
	validator_mem_region = request_mem_region(g_validator_base_addr, 
			PAGE_SIZE, "validator_hw_region");
	if(validator_mem_region == NULL) {
		pr_err("request_mem_region failed for VALIDATOR_PHYS_BASE\n");
		goto bad_exit_return;
	}
	
	g_ioremap_addr = ioremap(g_validator_base_addr, PAGE_SIZE);
	if(g_ioremap_addr == NULL) {
		pr_err("unable to ioremap VALIDATOR_PHYS_BASE\n");
		goto bad_exit_release_mem_region;
	}
 
	ret_val = driver_create_file(&the_platform_driver.driver, 
			driver_attribute_array[0]);
	if(ret_val != 0) {
		pr_err("driver_create_file returned %d\n", ret_val);
		ret_val = -EBUSY;
		goto bad_exit_iounmap;
	}

	ret_val = driver_create_file(&the_platform_driver.driver, 
			driver_attribute_array[1]);
	if(ret_val != 0) {
		pr_err("driver_create_file returned %d\n", ret_val);
		ret_val = -EBUSY;
		goto bad_exit_driver_remove_file_0;
	}

	ret_val = -EBUSY;
	if (misc_register (&validator_raw_dev_device)) {
		pr_warn ("Could not register device \"validator_raw\"...");
		goto bad_exit_driver_remove_file_1;
	}
	
	if (misc_register (&validator_sign_dev_device)) {
		pr_warn ("Could not register device \"validator_sign\"...");
		goto bad_exit_misc_deregister_raw;
	}
	
	if (misc_register (&validator_auth_dev_device)) {
		pr_warn ("Could not register device \"validator_auth\"...");
		goto bad_exit_misc_deregister_sign;
	}
	
	if (misc_register (&validator_rand_dev_device)) {
		pr_warn ("Could not register device \"validator_rand\"...");
		goto bad_exit_misc_deregister_auth;
	}
	
	if (misc_register (&validator_entropy_dev_device)) {
		pr_warn ("Could not register device \"validator_entropy\"...");
		goto bad_exit_misc_deregister_rand;
	}
	
	up(&g_driver_mutex);
	return 0;
	
bad_exit_misc_deregister_rand:
	misc_deregister (&validator_rand_dev_device);
bad_exit_misc_deregister_auth:
	misc_deregister (&validator_auth_dev_device);
bad_exit_misc_deregister_sign:
	misc_deregister (&validator_sign_dev_device);
bad_exit_misc_deregister_raw:
	misc_deregister (&validator_raw_dev_device);
bad_exit_driver_remove_file_1:
	driver_remove_file(&the_platform_driver.driver,
			driver_attribute_array[1]);
bad_exit_driver_remove_file_0:
	driver_remove_file(&the_platform_driver.driver,
			driver_attribute_array[0]);
bad_exit_iounmap:
	iounmap(g_ioremap_addr);
bad_exit_release_mem_region:
	release_mem_region(g_validator_base_addr, PAGE_SIZE);
bad_exit_return:
	return ret_val;
}

static int platform_remove(struct platform_device *pdev) {

	if(down_interruptible(&g_driver_mutex));

	misc_deregister (&validator_entropy_dev_device);
	misc_deregister (&validator_rand_dev_device);
	misc_deregister (&validator_auth_dev_device);
	misc_deregister (&validator_sign_dev_device);
	misc_deregister (&validator_raw_dev_device);
	driver_remove_file(&the_platform_driver.driver,
			driver_attribute_array[1]);
	driver_remove_file(&the_platform_driver.driver,
			driver_attribute_array[0]);
	iounmap(g_ioremap_addr);
	release_mem_region(g_validator_base_addr, PAGE_SIZE);
	return 0;
}

static int __init the_module_init(void) {

	int ret_val;
	
	ret_val = platform_driver_register(&the_platform_driver);
	if(ret_val != 0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		return(ret_val);
	}

        return 0;
}

static void __exit the_module_exit(void) {

	platform_driver_unregister(&the_platform_driver);

        return;
}

module_init(the_module_init);
module_exit(the_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RSF");
MODULE_DESCRIPTION("Provides numerous driver interfaces for the validator hardware block.");
MODULE_VERSION("1.0");

