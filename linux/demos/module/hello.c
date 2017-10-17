#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Halpern");
MODULE_VERSION("0.0.1");

static char *name = "World";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "Name to display in dmesg");

static struct miscdevice my_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "my time",
	.fops = &my_fops
};

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	//.open = my_open,
	//.release = my_close,
	.read = my_read
	//.llseek = noop_llseek
};


static ssize_t my_read(struct file *file, char __user *out, size_t size, loff_t *off)
{
	char *buf = vmalloc(size);
	sprintf(buf, "Hello World\n");
	if (copy_to_user(out, buf, strlen(buf)+1))
	{
		return -EFAULT;
	}
}


// called when module is installed
int __init init_module(void)
{
	printk(KERN_INFO "mymodule: Hello %s!\n", name);

	misc_register(&my_misc_device);

	return 0;
}



// called when module is removed
void __exit cleanup_module(void)
{
	printk(KERN_INFO "mymodule: Goodbye, cruel %s!!\n", name);

	misc_deregister(&my_misc_device);
}
