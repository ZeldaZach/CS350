#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Halpern");
MODULE_VERSION("0.0.1");

static ssize_t my_read(struct file *f, char __user *buffer, size_t length, loff_t *offset)
{
    int lnCopyToUser;
    char *lspOutBuffer = kzalloc(length, GFP_KERNEL);

    printk(KERN_INFO "Address %p", lspOutBuffer);

    if (! access_ok(VERIFY_WRITE, buffer, length))
    {
        printk(KERN_ERR "access_ok failed module_access_ok\n");
        return -EFAULT;
    }

    lCurrentTimeKernel = current_kernel_time();
    getnstimeofday(&lCurrentTimeDay);

    sprintf(lspOutBuffer,
        "current_kernel_time: %lu %lu\ngetnstimeofday: %lu %lu\n",
        lCurrentTimeKernel.tv_sec,
        lCurrentTimeKernel.tv_nsec,
        lCurrentTimeDay.tv_sec,
        lCurrentTimeDay.tv_nsec
    );

    lnCopyToUser = copy_to_user(buffer, lspOutBuffer, strlen(lspOutBuffer)+1);

    kfree(lspOutBuffer);
    if (lnCopyToUser > 0)
    {
        printk(KERN_ERR "copy_to_user failed mytime %d\n", lnCopyToUser);
        return -EFAULT;
    }

    return lnCopyToUser;
}

static ssize_t my_write(struct file *f, const char __user *buffer, size_t length, loff_t *offset)
{

}


static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read
    .write = my_write
};

static struct miscdevice my_numpipe_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "numpipe",
    .fops = &my_fops
};

// called when module is installed
int __init init_module(void)
{
    printk(KERN_INFO "numpipe: Loaded into Kernel\n");

    misc_register(&my_numpipe_device);

    return 0;
}



// called when module is removed
void __exit cleanup_module(void)
{
    printk(KERN_INFO "numpipe: Removed from Kernel\n");

    misc_deregister(&my_numpipe_device);
}
