#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


static struct class *FirstDrv_class;
static struct class_device	*FirstDrv_class_devs;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;


static int first_drv_open(struct inode *inode, struct file *file)
{
	

	//配置gpf456 为输出引脚

	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)) );
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));

	
	
    return 0;
}



static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);

	if(val == 1)
	{
		//点灯
		
        *gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
		    
	}
	else
	{
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}


	
	
    return 0;
}






/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,
	.write	=	first_drv_write,
};

int major;

int first_drv_init(void)
{
    major = register_chrdev(0,"first_drv", &first_drv_fops);
	
	FirstDrv_class = class_create(THIS_MODULE, "FirstDrv_class");
	if (IS_ERR(FirstDrv_class))
		return PTR_ERR(FirstDrv_class);
	FirstDrv_class_devs = class_device_create(FirstDrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/leds */

	if (unlikely(IS_ERR(FirstDrv_class_devs)))
		return PTR_ERR(FirstDrv_class_devs);

	gpfcon = (volatile unsigned long*)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	
	
    return 0;
}


void first_drv_exit(void)
{
    unregister_chrdev(major, "first_drv");
	class_device_unregister(FirstDrv_class_devs);
	class_destroy(FirstDrv_class);
	iounmap(gpfcon);
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");
