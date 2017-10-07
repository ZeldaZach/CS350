#include <linux/linkage.h>
#include <linux/export.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>

asmlinkage int sys_my_xtime(struct timespec *current_time)
{
	struct timespec lCurrentTime = current_kernel_time();

	// Check if pointer is a valid location to write to
	if (! access_ok(VERIFY_WRITE, current_time, sizeof(current_time)))
	{
		return -EFAULT;
	}

	// Attempt to copy value from xtime -> current_time
	if (! copy_to_user(current_time, &lCurrentTime, sizeof(lCurrentTime)))
	{
		// If the copy failed for some reason
		return -EFAULT;
	}

	printk(KERN_INFO "Current XTime: %ld\n", lCurrentTime.tv_sec*1000000000 + lCurrentTime.tv_nsec);

	return 0;
}
