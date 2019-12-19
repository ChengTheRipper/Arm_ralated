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


//查询方式按键驱动
//给sysfs提供更多信息
//让udev机制自动创建设备节点
//创建class提供设备信息
//看原理图以及芯片手册，如何配置寄存器使得按钮被设置为输入装填
//编程，将物理地址使用ioremap分配虚拟地址给系统


int major; // 主设备号
static struct class *Second_drv_class;
static struct class_device	*Second_drv_class_devs;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static int Second_drv_open(struct inode *inode, struct file *file)
{
    //配置gpf 0，2为输入
    *gpfcon  &= ~(0x3<<(0*2)|0x3<<(2*2));
    //配置gpg 3，11为输入
    *gpgcon  &= ~(0x3<<(3 * 2)|0x3<<(11 * 2));
    
    printk("open");
    return  0;
}

static ssize_t Second_drv_read (struct file *file, char __user *buf,size_t size, loff_t *ppos)
{

    /*返回四个引脚的电平*/
    unsigned char Key_val[4];
    int regval;

    if(size != sizeof(Key_val))
        return 0;

    //读取gpf0，2的值
    regval = *gpfdat;
    Key_val[0] = (regval & (0x1)<<0) ? 1 : 0;
    Key_val[1] = (regval & (0x1)<<2) ? 1 : 0;
    
    //读取gpg3，11的值
    regval = *gpgdat;
    Key_val[2] = (regval & (0x1)<<3) ? 1 : 0;
    Key_val[3] = (regval & (0x1)<<11) ? 1 : 0;



    copy_to_user(buf, Key_val, sizeof(Key_val));

    return sizeof(Key_val);
}


//放入驱动身份结构体
/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations Second_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   Second_drv_open,
    .read   =   Second_drv_read,
	
};


//设备登记

int Second_drv_init(void)
{
    major = register_chrdev(0,"Second_drv", &Second_drv_fops);
	
    Second_drv_class = class_create(THIS_MODULE, "Second_drv_class");

	Second_drv_class_devs = class_device_create(Second_drv_class, NULL, MKDEV(major, 0), NULL, "buttom"); /* /dev/button */
    
    
    gpfcon = (volatile unsigned long*)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

    gpgcon = (volatile unsigned long*)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;
    
    printk("installed");
    return 0;
}

//设备卸载

void Second_drv_exit(void)
{
    unregister_chrdev(major, "Second_drv");
	class_device_unregister(Second_drv_class_devs);
	class_destroy(Second_drv_class);

    iounmap(gpfcon);
    iounmap(gpgcon);
	
}

module_init(Second_drv_init);
module_exit(Second_drv_exit);
MODULE_LICENSE("GPL");
