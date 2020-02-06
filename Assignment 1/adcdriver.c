#include<linux/init.h>
#include<linux/module.h>
#include<linux/version.h>
#include<linux/kernel.h>
#include<linux/types.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/random.h>
#include <linux/uaccess.h>
#include "adcchardev.h"

#define SUCCESS 0


static dev_t adc;
static struct cdev c_dev;
static struct class *cls;
static uint16_t ch = 0;
static char al = 'r';
static int Device_Open = 0;
uint16_t randomgenerator(void);


static int adc_open(struct inode *i,struct file *f){
		if (Device_Open)
        return -EBUSY;

    Device_Open++;
		return SUCCESS;
	}


static int adc_close(struct inode *i,struct file *f){
		Device_Open--;
		return SUCCESS;

	}


static ssize_t adc_read(struct file *f, char __user *buf, size_t len, loff_t *off){
    int missingbits;
    uint16_t randno;
    randno = randomgenerator();
		printk(KERN_INFO " random number generated by driver is %d",randno);
		if(al == 'l'){									// for left alignment
				randno = randno * 64;				// one bit left shift is equal to *2 so 6 bit leftshift is *64
		}
		printk(KERN_INFO "Current channel being used is %d and data alignment %c ",ch,al);
    missingbits = copy_to_user(buf,&randno,sizeof(randno));
    if(missingbits!=0){
    printk(KERN_INFO "Failed to copy %d byte(s) to the userspace ",missingbits);
    }
		return sizeof(randno);

	}


long adc_ioctl(struct file *file,unsigned int ioctl_num,unsigned long ioctl_param){

                switch(ioctl_num){
                  case SEL_CHANNEL:
										ch = ioctl_param;
										//printk(KERN_INFO "current request code from channel is %u",ioctl_num);
                    printk(KERN_INFO "CHANNEL SELECTED IS %d",ch);
										break;

                  case SEL_ALIGNMENT:
                    al = (char)ioctl_param;
										//printk(KERN_INFO "current request code from alignment is %u",ioctl_num);
                    printk(KERN_INFO "ALLIGNMENT SELECTED - %c",al);
										break;
                }
              return SUCCESS;
  }

uint16_t randomgenerator(void){
    unsigned int num;
    get_random_bytes(&num, 2);
    num%=1023;
    return (uint16_t)num;
  }

static struct file_operations fops =
																			{
																					.owner = THIS_MODULE,
																					.open = adc_open,
																					.release = adc_close,
                                          .unlocked_ioctl = adc_ioctl,
																					.read = adc_read
																			};

__init int adcdriver_init(void){
	// Step 1: Allocation of major and minor numbers.
	if(alloc_chrdev_region(&adc,0,5,"Akshay")<0){
		return -1;
	}
	printk(KERN_INFO "<major , minor>:<%d,%d>\n",MAJOR(adc),MINOR(adc));// to see the alloted major and minor no.

	// Step 2: Creation of device file.
	if((cls=class_create(THIS_MODULE,"adcclass"))==NULL){
		printk(KERN_INFO "Failed to create class......exiting");
			unregister_chrdev_region(adc,1);
			return -1;
	}
	if((device_create(cls,NULL,adc,NULL,"adc8"))==NULL){
		printk(KERN_INFO "Failed to create device......exiting");
			class_destroy(cls);
			unregister_chrdev_region(adc,1);
			return -1;
	}
	//Step 3: Link fops and cdev to device node;
	cdev_init(&c_dev,&fops);
	//making the driver live
	if(cdev_add(&c_dev,adc, 1)==-1){
		device_destroy(cls,adc);
		class_destroy(cls);
		unregister_chrdev_region(adc,1);
			printk(KERN_INFO "Failed to make driver live ......exiting");
		return -1;
	}
	printk(KERN_INFO "Inserted successfully Congratulations");
	return SUCCESS;
}


__exit void adcdriver_exit(void){
	cdev_del(&c_dev);
	device_destroy(cls,adc);
	class_destroy(cls);
	printk(KERN_INFO "entered in exit routine");
	unregister_chrdev_region(adc,3);
	printk(KERN_INFO "Deleted successfully Sab sahi hai");
}


module_init(adcdriver_init);
module_exit(adcdriver_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("10 bit 8 channel adc");
MODULE_AUTHOR("AKSHAY A SONI <h20190117@pilani.bits-pilani.ac.in>");
