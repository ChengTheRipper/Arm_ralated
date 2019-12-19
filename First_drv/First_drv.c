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
	

	//����gpf456 Ϊ�������

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
		//���
		
        *gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
		    
	}
	else
	{
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}


	
	
    return 0;
}






/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
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
