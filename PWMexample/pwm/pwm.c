/*
  pwm.c: Linux driver for a PWM on IO13
  
  Copyright (c) 2016, 2017 Walter Fetter Lages <w.fetter@ieee.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    You can also obtain a copy of the GNU General Public License
    at <http://www.gnu.org/licenses>.

*/

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "pwm"
#define CLASS_NAME "eng10032"

static int major=0;
static int minor=0;
static struct class *pwmclass=NULL;

MODULE_AUTHOR("Laurien Santin e Luis Miguel Batista");
MODULE_DESCRIPTION("pwm Driver");
MODULE_SUPPORTED_DEVICE("pwm");
MODULE_LICENSE("GPL");

module_param(major,int,0);
MODULE_PARM_DESC(major,"major number; default is kernel allocated.");

static ssize_t pwm_write(struct file *file,const char *buf,
	size_t count,loff_t *ppos)
{
	unsigned char data;
	int error;

	if(count != sizeof(unsigned char)) return -EINVAL;
	if( (error=get_user(data,buf)) ) return error;	
	gpio_set_value(7,data & 1);
	return sizeof(unsigned char);
}

static struct gpio routers[]={
	{46,GPIOF_OUT_INIT_LOW|GPIOF_EXPORT_DIR_FIXED,"IO13 is GPIO"},
	{30,GPIOF_OUT_INIT_LOW|GPIOF_EXPORT_DIR_FIXED,"IO13 is Out"},
	{7,GPIOF_OUT_INIT_HIGH|GPIOF_EXPORT_DIR_FIXED,"IO13"}
};

static int pwm_open(struct inode *inode,struct file *file)
{
	return gpio_request_array(routers,ARRAY_SIZE(routers));
}

static int pwm_release(struct inode *inode,struct file *file)
{
	gpio_free_array(routers,ARRAY_SIZE(routers));
	return 0;
}

static struct file_operations pwm_fops=
{
	.owner=THIS_MODULE,
	.write=pwm_write,
	.open=pwm_open,
	.release=pwm_release
};

int init_module(void)
{
	dev_t pwmno;

	if((major=register_chrdev(major,DEVICE_NAME,&pwm_fops))==-1)
	{
		printk("major %d can't be registered.\n",major);
		return -EIO;
	}
	pwmclass=class_create(THIS_MODULE,CLASS_NAME);
	pwmno=MKDEV(major,minor);
	device_create(pwmclass,NULL,pwmno,NULL,DEVICE_NAME "%d",minor);
	
	return 0;
}

void cleanup_module(void)
{
	dev_t pwmno;

	pwmno=MKDEV(major,minor);
	device_destroy(pwmclass,pwmno);
	if(pwmclass != NULL) class_destroy(pwmclass);
	unregister_chrdev(major,DEVICE_NAME);
}
