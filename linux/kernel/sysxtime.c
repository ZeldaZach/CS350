#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>

asmlinkage int sys_xtime(void)
{
	printk(KERN_ALERT "XTimeDemo\n");
	return 0;
}
