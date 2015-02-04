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
#include <asm/io.h>

// prototypes
static struct platform_driver the_platform_driver;

// globals
static int	g_sld_hub_csr_base_addr;
static void    *g_ioremap_addr;

static ssize_t csr_show(struct device_driver *driver, char *buf) {
	uint32_t raw_status;
	char *response;
	
	raw_status = ioread32(g_ioremap_addr);
	raw_status &= 0x01;

	if(raw_status == 0)
		response = "JTAG PINS controls FPGA SLD HUB";
	else {
		response = "FPGA logic controls FPGA SLD HUB";
	}
	
	return scnprintf(buf, PAGE_SIZE,
		"%s\n",
		response);
}

static ssize_t csr_store(struct device_driver *driver, const char *buf,
		size_t count) {
	int result;
	unsigned long new_value;
	uint32_t raw_status;
	
	// convert the input string to the requested new value
	result = kstrtoul(buf, 0, &new_value);
	if(result != 0)
		return(-EINVAL);
	
	// range check the requested new value
	if(new_value > 1)
		return(-EINVAL);
		
	raw_status = ioread32(g_ioremap_addr);
	raw_status &= 0xFFFFFFFE;
	raw_status |= new_value;
	
	// write the new value
	iowrite32(raw_status, g_ioremap_addr);
	
	return(count);
}
DRIVER_ATTR(csr, (S_IWUGO | S_IRUGO), csr_show, csr_store);

static int platform_probe(struct platform_device *pdev) {
	int ret_val;
	struct resource *r;
	struct resource *sld_hub_csr_mem_region;
	
	ret_val = -EINVAL;

	// get our first memory resource
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(r != NULL) {
		if(r->start & ~PAGE_MASK) {
			pr_err("sld_hub_csr base not aligned to PAGE_SIZE\n");
			goto bad_exit_return;
		}
		if(((r->end - r->start) + 1) > PAGE_SIZE) {
			pr_err("sld_hub_csr span is larger than PAGE_SIZE\n");
			goto bad_exit_return;
		}
	} else {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		goto bad_exit_return;
	}

	g_sld_hub_csr_base_addr = r->start;

	ret_val = -EBUSY;

	// reserve our memory region
	sld_hub_csr_mem_region = request_mem_region(g_sld_hub_csr_base_addr, 
			((r->end - r->start) + 1), "sld_hub_csr_hw_region");
	if(sld_hub_csr_mem_region == NULL) {
		pr_err("request_mem_region failed: g_sld_hub_csr_base_addr\n");
		goto bad_exit_return;
	}
	
	// ioremap our memory region
	g_ioremap_addr = ioremap(g_sld_hub_csr_base_addr, PAGE_SIZE);
	if(g_ioremap_addr == NULL) {
		pr_err("ioremap failed: g_sld_hub_csr_base_addr\n");
		goto bad_exit_release_mem_region;
	}
	
	// create the sysfs entry
	ret_val = driver_create_file(&the_platform_driver.driver,
			&driver_attr_csr);
	if(ret_val != 0) {
		pr_err("failed to create csr sysfs entry");
		goto bad_exit_iounmap;
	}
	
	return 0;

bad_exit_iounmap:
	iounmap(g_ioremap_addr);
bad_exit_release_mem_region:
	release_mem_region(g_sld_hub_csr_base_addr, PAGE_SIZE);
bad_exit_return:
	return ret_val;
}

static int platform_remove(struct platform_device *pdev) {
	driver_remove_file(&the_platform_driver.driver, &driver_attr_csr);
	iounmap(g_ioremap_addr);
	release_mem_region(g_sld_hub_csr_base_addr, PAGE_SIZE);

	return 0;
}

static struct of_device_id sld_hub_csr_driver_dt_ids[] = {
	{
		.compatible = "altr,sld_hub_csr-14.1"
	},
	{ /* end of table */ }
};
MODULE_DEVICE_TABLE(of, sld_hub_csr_driver_dt_ids);

static struct platform_driver the_platform_driver = {
	.probe = platform_probe,
	.remove = platform_remove,
	.driver = {
		.name = "sld_hub_csr",
		.owner = THIS_MODULE,
		.of_match_table = sld_hub_csr_driver_dt_ids,
	},
};

static int sld_hub_csr_init(void) {
	int ret_val;
	
	ret_val = platform_driver_register(&the_platform_driver);
	if(ret_val != 0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		return(ret_val);
	}

	return(0);
}

static void sld_hub_csr_exit(void) {
	platform_driver_unregister(&the_platform_driver);
}

module_init(sld_hub_csr_init);
module_exit(sld_hub_csr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RSF");
MODULE_DESCRIPTION("Driver to provide sysfs access to sld_hub_csr");
MODULE_VERSION("1.0");

