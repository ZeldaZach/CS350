#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>

asmlinkage int sys_my_xtime(struct timespec *current_time)
{
	struct timespec lCurrentTime;
	int lCopy2UserStatus;

	// Check if pointer is a valid location to write to
	if (! access_ok(VERIFY_WRITE, current_time, sizeof(current_time)))
	{
		printk(KERN_ERR "access_ok failed sys_my_xtime\n");
		return -EFAULT;
	}

	// Attempt to copy value from xtime -> current_time
	lCurrentTime = current_kernel_time();
	lCopy2UserStatus = copy_to_user(current_time, &lCurrentTime, sizeof(lCurrentTime));

	if (lCopy2UserStatus > 0)
	{
		// If the copy failed for some reason
		printk(KERN_ERR "copy_to_user failed sys_my_xtime: %d\n", lCopy2UserStatus);
		return -EFAULT;
	}

	//printk(KERN_INFO "current_kernel_time: %ld %ld\n", lCurrentTime.tv_sec, lCurrentTime.tv_nsec);
	printk(KERN_INFO "current time in NS: %ld\n", lCurrentTime.tv_sec*1000000000UL + lCurrentTime.tv_nsec);

	return 0;
}

EXPORT_SYMBOL(sys_my_xtime);
