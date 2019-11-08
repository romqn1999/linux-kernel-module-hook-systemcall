#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/random.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/kdev_t.h>

#define DRIVER_AUTHOR "Pham Hoang Nhat Anh phamhoangnhatanh2707@gmail.com"
#define DRIVER_DESC "Random number generator"

static int randomNumber_open(struct inode *i, struct file *f);
static ssize_t randomNumber_read(struct file *filp, char *buffer, size_t length, loff_t *offset);
static int randomNumber_close(struct inode *i, struct file *f);

static int Major;
static dev_t dev_num;
static struct class * dev_class;
static struct device * dev;
static struct cdev c_dev;
static struct file_operations fops={
    .owner = THIS_MODULE,
    .open = randomNumber_open,
    .read = randomNumber_read,
    .release = randomNumber_close,  
};

static char *RandomNumber_devnode(struct device *dev, umode_t *mode)
{
        if (!mode)
                return NULL;
        * mode = 0666;
        return NULL;
}

static int randomNumber_init(void)
{
    int ret;

    ret = alloc_chrdev_region(& dev_num, 0, 1, "RandomNumber");
    
    Major = MAJOR(dev_num); 

    if(ret < 0) {
    	printk(KERN_INFO "RandomNumber: Registration failed\n");
    }
    else {
        printk(KERN_INFO "RandomNumber: Successful device registration with MajorID = %d\n", Major);
        dev_class = class_create(THIS_MODULE, "Class_RandomNumber");
        if (dev_class == NULL) {
            printk(KERN_INFO "RandomNumber: Failed to create a device class\n");
            unregister_chrdev_region(dev_num, 1);;  
            return 0;
        }    
        dev_class->devnode = RandomNumber_devnode;  //permission
        dev = device_create(dev_class, NULL, dev_num, NULL, "RandomNumber");
        if (IS_ERR(dev)){
            printk(KERN_INFO "RandomNumber: Failed to create a device\n");
            class_destroy(dev_class);
            unregister_chrdev_region(dev_num, 1); 
            return 0;
        }
        cdev_init(& c_dev, & fops);
        if (cdev_add(&c_dev, dev_num, 1) == -1) {
            device_destroy(dev_class, dev_num);
            class_destroy(dev_class);
            unregister_chrdev_region(dev_num, 1);
            return 0;
        }

        printk(KERN_INFO "RandomNumber: Initialize RandomNumber driver successfully\n");
    }
    return 0;
}

static int randomNumber_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "RandomNumber: open()\n");
    return 0;
}

static ssize_t randomNumber_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    printk(KERN_INFO "RandomNumber: read()\n");
    return get_random_int(); 
}

static int randomNumber_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "RandomNumber: close()\n");
    return 0;
}

static void randomNumber_exit(void)
{
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    unregister_chrdev_region(dev_num, 1);      
    printk(KERN_INFO "RandomNumber: The device has been disconected from the system\n");
}

module_init(randomNumber_init);
module_exit(randomNumber_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR); 
MODULE_DESCRIPTION(DRIVER_DESC);
