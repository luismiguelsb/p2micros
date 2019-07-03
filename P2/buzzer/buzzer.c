//DUPLA: Laurien Santin e Luis Miguel Batista

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>

#define DEVICE_NAME "eng10032-buzzer"
#define CLASS_NAME "eng10032"

static int major=63;
static int minor=1;
static struct class *buzzerclass=NULL;
struct pwm_device* pwm = NULL;

MODULE_AUTHOR("Laurien Santin e Luis Miguel Batista");
MODULE_DESCRIPTION("buzzer Driver");
MODULE_SUPPORTED_DEVICE("buzzer");
MODULE_LICENSE("GPL");

module_param(major,int,0);
MODULE_PARM_DESC(major,"major number; default is kernel allocated.");

static ssize_t buzzer_write(struct file *file,const char *buf,
	size_t count,loff_t *ppos)
{
	unsigned char data[3];
	int error;
	int freq, duty_cycle;

	if(count != sizeof(unsigned char)) return -EINVAL;
	if( (error=copy_from_user(data,buf,3)) ) return error;
	//A compilação não estava reconhecendo bibliotecas como stdio.h e stdlib.hh, então implementamos um atoi para 3 bytes
	freq = (data[0] - 48) * 1000 + (data[1] - 48) * 10 + (data[2] - 48);
	duty_cycle = ((1 / freq) * 1000000000) / 2;
	pwm_config(pwm,duty_cycle,freq);
	pwm_enable(pwm);
	return sizeof(unsigned char);
}

static ssize_t buzzer_read(struct file *file, const char *buf, size_t count,loff_t *ppos)
{
	int period;
	char ascii[4];
	if(count != sizeof(unsigned char)) return -EINVAL;
	period = pwm_get_period(pwm);
	ascii[0] = period / 1000 + 48;
	ascii[1] = period > 1000 ? (period - 1000) / 10 : period / 10;
	ascii[2] = period % 10;
	ascii[3] = '\0';
	return copy_to_user(ascii, buf, ARRAY_SIZE(ascii));
	//return sizeof(ascii);
}

static struct gpio routers[]={
	{20,GPIOF_OUT_INIT_LOW|GPIOF_EXPORT_DIR_FIXED,"IO6 is PWM"},
	{21,GPIOF_IN|GPIOF_EXPORT_DIR_FIXED,"IO6 is In"},
	{68,GPIOF_OUT_INIT_HIGH|GPIOF_EXPORT_DIR_FIXED,"IO6"}
};

static int buzzer_open(struct inode *inode,struct file *file)
{
	pwm = pwm_request(5,"eng10032-buzzer");
	return gpio_request_array(routers, ARRAY_SIZE(routers));
}

static int buzzer_release(struct pwm_device *pwm)
{
	gpio_free_array(routers, ARRAY_SIZE(routers));
	pwm_free(pwm);
	return 0;
}

static struct file_operations buzzer_fops=
{
	.owner=THIS_MODULE,
	.write=buzzer_write,
	.read=buzzer_read,
	.open=buzzer_open,
	.release=buzzer_release
};

int init_module(void)
{
	dev_t buzzerno;

	if((major=register_chrdev(major,DEVICE_NAME,&buzzer_fops))==-1)
	{
		printk("major %d can't be registered.\n",major);
		return -EIO;
	}
	buzzerclass=class_create(THIS_MODULE,CLASS_NAME);
	buzzerno=MKDEV(major,minor);
	device_create(buzzerclass,NULL,buzzerno,NULL,DEVICE_NAME "%d",minor);
	
	return 0;
}

void cleanup_module(void)
{
	dev_t buzzerno;

	buzzerno=MKDEV(major,minor);
	device_destroy(buzzerclass,buzzerno);
	if(buzzerclass != NULL) class_destroy(buzzerclass);
	unregister_chrdev(major,DEVICE_NAME);
}
