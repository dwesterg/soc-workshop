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
#include "demo_module_11.h"

/* globals */
static uint32_t g_target;

static ssize_t target_show(struct device_driver *driver, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "0x%08X\n", g_target);
}

static ssize_t target_store(struct device_driver *driver, const char *buf,
			    size_t count)
{
	int result;
	unsigned long new_target;

	/* convert the input string to the requested new target value */
	result = kstrtoul(buf, 0, &new_target);
	if (result != 0)
		return -EINVAL;

	g_target = new_target;

	return count;
}

DRIVER_ATTR(target, (S_IWUGO | S_IRUGO), target_show, target_store);

static ssize_t increment_show(struct device_driver *driver, char *buf)
{
	demo_increment(&g_target);

	return scnprintf(buf, PAGE_SIZE, "0x%08X\n", g_target);
}

DRIVER_ATTR(increment, (S_IRUGO), increment_show, NULL);

static ssize_t decrement_show(struct device_driver *driver, char *buf)
{
	demo_decrement(&g_target);

	return scnprintf(buf, PAGE_SIZE, "0x%08X\n", g_target);
}

DRIVER_ATTR(decrement, (S_IRUGO), decrement_show, NULL);

static ssize_t complement_show(struct device_driver *driver, char *buf)
{
	demo_complement(&g_target);

	return scnprintf(buf, PAGE_SIZE, "0x%08X\n", g_target);
}

DRIVER_ATTR(complement, (S_IRUGO), complement_show, NULL);

static struct of_device_id demo_driver_dt_ids[] = {
	{
	 .compatible = "demo,custom_api"},
	{ /* end of table */ }
};

MODULE_DEVICE_TABLE(of, demo_driver_dt_ids);

static struct platform_driver the_platform_driver = {
	.driver = {
		   .name = "demo_driver_11t",
		   .owner = THIS_MODULE,
		   .of_match_table = demo_driver_dt_ids,
		   },
};

static int demo_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&the_platform_driver);
	if (ret_val != 0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		goto bad_exit_return;
	}
	/* create the sysfs entries */
	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_target);
	if (ret_val != 0) {
		pr_err("failed to create 'target' sysfs entry");
		goto bad_exit_platform_driver_unregister;
	}

	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_increment);
	if (ret_val != 0) {
		pr_err("failed to create 'increment' sysfs entry");
		goto bad_exit_remove_target_file;
	}

	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_decrement);
	if (ret_val != 0) {
		pr_err("failed to create 'decrement' sysfs entry");
		goto bad_exit_remove_increment_file;
	}

	ret_val = driver_create_file(&the_platform_driver.driver,
				     &driver_attr_complement);
	if (ret_val != 0) {
		pr_err("failed to create 'complement' sysfs entry");
		goto bad_exit_remove_decrement_file;
	}

	pr_info("demo_init exit\n");
	return 0;

bad_exit_remove_decrement_file:
	driver_remove_file(&the_platform_driver.driver, &driver_attr_decrement);
bad_exit_remove_increment_file:
	driver_remove_file(&the_platform_driver.driver, &driver_attr_increment);
bad_exit_remove_target_file:
	driver_remove_file(&the_platform_driver.driver, &driver_attr_target);
bad_exit_platform_driver_unregister:
	platform_driver_unregister(&the_platform_driver);
bad_exit_return:
	return ret_val;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");

	driver_remove_file(&the_platform_driver.driver,
			   &driver_attr_complement);
	driver_remove_file(&the_platform_driver.driver, &driver_attr_decrement);
	driver_remove_file(&the_platform_driver.driver, &driver_attr_increment);
	driver_remove_file(&the_platform_driver.driver, &driver_attr_target);
	platform_driver_unregister(&the_platform_driver);

	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_DESCRIPTION("Demonstration Module 11t - test custom API");
MODULE_VERSION("1.0");
