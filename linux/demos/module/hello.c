#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Halpern");
MODULE_VERSION("0.0.1");

static char *name = "World";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "Name to display in dmesg");


static ssize_t my_read(struct file *file, char __user *out, size_t size, loff_t *off)
{
	int lnCopyToUser;
	char *buf = kzalloc(size, GFP_USER);

    if (! access_ok(VERIFY_WRITE, buf, sizeof(buf)))
	{
		printk(KERN_ERR "access_ok failed module_access_ok\n");
		return -EFAULT;
	}

	sprintf(buf, "Hello World\n");

	lnCopyToUser = copy_to_user(out, buf, strlen(buf)+1);
	if (lnCopyToUser > 0)
	{
		kfree(buf);
		return -EFAULT;
	}

	kfree(buf);

	return size;
}

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = my_read
};

static struct miscdevice my_time_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "my time",
	.fops = &my_fops
};

// called when module is installed
int __init init_module(void)
{
	printk(KERN_INFO "mymodule: Hello %s!\n", name);

	misc_register(&my_time_device);

	return 0;
}



// called when module is removed
void __exit cleanup_module(void)
{
	printk(KERN_INFO "mymodule: Goodbye, cruel %s!!\n", name);

	misc_deregister(&my_time_device);
}
