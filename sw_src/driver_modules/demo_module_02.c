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

/* define module parameters */
static unsigned char param_byte = 0xFF;
static short param_short = 0xFFFF;
static unsigned short param_ushort = 0xFFFF;
static int param_int = 0xFFFFFFFF;
static unsigned int param_uint = 0xFFFFFFFF;
static long param_long = 0xFFFFFFFF;
static unsigned long param_ulong = 0xFFFFFFFF;
static bool param_bool = 1;
static char *param_charp;

module_param(param_byte, byte, S_IRUGO);
module_param(param_short, short, S_IRUGO);
module_param(param_ushort, ushort, S_IRUGO);
module_param(param_int, int, S_IRUGO);
module_param(param_uint, uint, S_IRUGO);
module_param(param_long, long, S_IRUGO);
module_param(param_ulong, ulong, S_IRUGO);
module_param(param_bool, bool, S_IRUGO);
module_param(param_charp, charp, S_IRUGO);

MODULE_PARM_DESC(param_byte, "a byte parameter");
MODULE_PARM_DESC(param_short, "a short parameter");
MODULE_PARM_DESC(param_ushort, "a ushort parameter");
MODULE_PARM_DESC(param_int, "a int parameter");
MODULE_PARM_DESC(param_uint, "a uint parameter");
MODULE_PARM_DESC(param_long, "a long parameter");
MODULE_PARM_DESC(param_ulong, "a ulong parameter");
MODULE_PARM_DESC(param_bool, "a bool parameter");
MODULE_PARM_DESC(param_charp, "a charp parameter");

static int param_int_array[] = { 1, 2, 3, 4 };

static unsigned int param_int_array_count;

module_param_array(param_int_array, int, &param_int_array_count, S_IRUGO);

MODULE_PARM_DESC(param_int_array, "an array of int parameters");

static int demo_init(void)
{
	pr_info("demo_init enter\n");

	pr_info("\n");
	pr_info("param_byte   = 0x%02X : %u\n", param_byte, param_byte);
	pr_info("param_short  = 0x%04X : %d\n", param_short, param_short);
	pr_info("param_ushort = 0x%04X : %u\n", param_ushort, param_ushort);
	pr_info("param_int    = 0x%08X : %d\n", param_int, param_int);
	pr_info("param_uint   = 0x%08X : %u\n", param_uint, param_uint);
	pr_info("param_long   = 0x%08lX : %ld\n", param_long, param_long);
	pr_info("param_ulong  = 0x%08lX : %lu\n", param_ulong, param_ulong);
	pr_info("param_bool   = %d\n", param_bool);
	pr_info("param_charp  = \'%s\'\n", param_charp);
	pr_info("\n");
	pr_info("param_int_array_count = %d\n", param_int_array_count);
	pr_info("param_int_array[0] = %d\n", param_int_array[0]);
	pr_info("param_int_array[1] = %d\n", param_int_array[1]);
	pr_info("param_int_array[2] = %d\n", param_int_array[2]);
	pr_info("param_int_array[3] = %d\n", param_int_array[3]);
	pr_info("\n");

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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Driver Student One <dso@company.com>");
MODULE_DESCRIPTION("Demonstration Module 2 - introduce module parameters");
MODULE_VERSION("1.0");
