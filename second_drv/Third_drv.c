#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>




//��ѯ��ʽ��������
//��sysfs�ṩ������Ϣ
//��udev�����Զ������豸�ڵ�
//����class�ṩ�豸��Ϣ
//��ԭ��ͼ�Լ�оƬ�ֲᣬ������üĴ���ʹ�ð�ť������Ϊ����װ��
//��̣��������ַʹ��ioremap���������ַ��ϵͳ


int major; // ���豸��
static struct class *Third_drv_class;
static struct class_device	*Third_drv_class_devs;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static DECLARE_WAIT_QUEUE_HEAD(buttoms_waitq);

/* �ж��¼���־, �жϷ����������1��third_drv_read������0 */
static volatile int ev_press = 0;



struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};
//����ʱ��ֵ��0x01,0x02,0x03,0x04
//�ɿ�ʱ��ֵ��0x81,0x82,0x83,0x84

static unsigned char key_val;

struct pin_desc Pin_D[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04}
};




//����������ֵ
static irqreturn_t buttoms_irq(int irq, void *dev_id)
{
	
	//pin_desc * tmppin = (pin_desc *)dev_id;
	struct pin_desc * tmppin = (struct pin_desc *)dev_id;
	unsigned int  pinval;
	pinval = s3c2410_gpio_getpin(tmppin->pin);
	if(pinval)
	{
	    //�ɿ�
	    key_val = 0x80 | tmppin->key_val;
	}
	else
	{
	    //����
	    key_val = tmppin->key_val;
	}
	ev_press = 1;
	wake_up_interruptible(&buttoms_waitq);

	return IRQ_RETVAL(IRQ_HANDLED);
	
	// struct pin_desc * pindesc = (struct pin_desc *)dev_id;
	// unsigned int pinval;

	// pinval = s3c2410_gpio_getpin(pindesc->pin);

	// if (pinval)
	// {
	// 	/* �ɿ� */
	// 	key_val = 0x80 | pindesc->key_val;
	// }
	// else
	// {
	// 	/* ���� */
	// 	key_val = pindesc->key_val;
	// }

    // ev_press = 1;                  /* ��ʾ�жϷ����� */
    // wake_up_interruptible(&buttoms_waitq);   /* �������ߵĽ��� */


	// return IRQ_RETVAL(IRQ_HANDLED);
}



static int Third_drv_open(struct inode *inode, struct file *file)
{
    //����gpf 0��2Ϊ�ж�

    //����gpg 3��11Ϊ����

	request_irq(IRQ_EINT0,  buttoms_irq, IRQT_BOTHEDGE, "S2", &Pin_D[0]);
    request_irq(IRQ_EINT2,  buttoms_irq, IRQT_BOTHEDGE, "S3", &Pin_D[1]);
    // request_irq(IRQ_EINT19, buttoms_irq, IRQT_BOTHEDGE, "S4", &Pin_D[2]);
    // request_irq(IRQ_EINT11, buttoms_irq, IRQT_BOTHEDGE, "S5", &Pin_D[3]);
	request_irq(IRQ_EINT11, buttoms_irq, IRQT_BOTHEDGE, "S4", &Pin_D[2]);
    request_irq(IRQ_EINT19, buttoms_irq, IRQT_BOTHEDGE, "S5", &Pin_D[3]);
    printk("\nopen");
    return  0;
}

static ssize_t Third_drv_read (struct file *file, char __user *buf,size_t size, loff_t *ppos)
{

    if (size != 1)
    	return -EINVAL;

	//���û�а����������ߣ�������
	wait_event_interruptible(buttoms_waitq, ev_press);
	//����а��������������ؼ�ֵ

    copy_to_user(buf, &key_val, 1);
    ev_press = 0;

    return 1;
}

int Third_drv_close(struct inode *inode, struct file *file)
{
    free_irq(IRQ_EINT0, &Pin_D[0]);
	free_irq(IRQ_EINT2, &Pin_D[1]);
	// free_irq(IRQ_EINT19, &Pin_D[2]);
	// free_irq(IRQ_EINT11, &Pin_D[3]);
	free_irq(IRQ_EINT11, &Pin_D[2]);
	free_irq(IRQ_EINT19, &Pin_D[3]);
	return 0;
}


//����������ݽṹ��
/* ����ṹ���ַ��豸��������ĺ���
 * ��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ�����
 * ���ջ��������ṹ��ָ���Ķ�Ӧ����
 */
static struct file_operations Third_drv_fops = {
    .owner   =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =   Third_drv_open,
    .read    =   Third_drv_read,
    .release =   Third_drv_close,

};


//�豸�Ǽ�

int Third_drv_init(void)
{
    major = register_chrdev(0,"Third_drv", &Third_drv_fops);

    Third_drv_class = class_create(THIS_MODULE, "Third_drv_class");

	Third_drv_class_devs = class_device_create(Third_drv_class, NULL, MKDEV(major, 0), NULL, "buttom"); /* /dev/button */


    gpfcon = (volatile unsigned long*)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

    gpgcon = (volatile unsigned long*)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

    printk("installed");
    return 0;
}

//�豸ж��

void Third_drv_exit(void)
{
    unregister_chrdev(major, "Third_drv");
	class_device_unregister(Third_drv_class_devs);
	class_destroy(Third_drv_class);

    iounmap(gpfcon);
    iounmap(gpgcon);

}

module_init(Third_drv_init);
module_exit(Third_drv_exit);
MODULE_LICENSE("GPL");

