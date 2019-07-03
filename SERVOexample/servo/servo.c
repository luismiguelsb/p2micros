//DUPLA: Laurien Santin e Luis Miguel Batista

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "servo"
#define CLASS_NAME "eng10032"

static int major=0;
static int minor=0;
static struct class *servoclass=NULL;

MODULE_AUTHOR("Laurien Santin e Luis Miguel Batista");
MODULE_DESCRIPTION("servo Driver");
MODULE_SUPPORTED_DEVICE("servo");
MODULE_LICENSE("GPL");

module_param(major,int,0);
MODULE_PARM_DESC(major,"major number; default is kernel allocated.");

static ssize_t servo_write(struct file *file,const char *buf,
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

static int servo_open(struct inode *inode,struct file *file)
{
	return gpio_request_array(routers,ARRAY_SIZE(routers));
}

static int servo_release(struct inode *inode,struct file *file)
{
	gpio_free_array(routers,ARRAY_SIZE(routers));
	return 0;
}

static struct file_operations servo_fops=
{
	.owner=THIS_MODULE,
	.write=servo_write,
	.open=servo_open,
	.release=servo_release
};

int init_module(void)
{
	dev_t servono;

	if((major=register_chrdev(major,DEVICE_NAME,&servo_fops))==-1)
	{
		printk("major %d can't be registered.\n",major);
		return -EIO;
	}
	servoclass=class_create(THIS_MODULE,CLASS_NAME);
	servono=MKDEV(major,minor);
	device_create(servoclass,NULL,servono,NULL,DEVICE_NAME "%d",minor);
	
	return 0;
}

void cleanup_module(void)
{
	dev_t servono;

	servono=MKDEV(major,minor);
	device_destroy(servoclass,servono);
	if(servoclass != NULL) class_destroy(servoclass);
	unregister_chrdev(major,DEVICE_NAME);
}
