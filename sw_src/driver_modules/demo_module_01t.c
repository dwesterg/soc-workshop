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

static int demo_init(void)
{
	pr_info("demo_init enter\n");
	pr_info("demo_init exit\n");
	return 0;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");
	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

/*
MODULE_LICENSE("GPL");
*/
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_AUTHOR("Driver Student Two <dst@organization.org>");
MODULE_DESCRIPTION("Demonstration Module 1t - tainted module example");
MODULE_VERSION("1.0");
