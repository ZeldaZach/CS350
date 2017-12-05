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
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zach Halpern");
MODULE_VERSION("0.0.2");

/* Header Start */
int __init init_module(void);
void __exit cleanup_module(void);
static ssize_t my_read(struct file *f, char __user *buffer, size_t length, loff_t *offset);
static ssize_t my_write(struct file *f, const char __user *buffer, size_t length, loff_t *offset);
int fifo_is_full(void);
int fifo_is_empty(void);
int fifo_push(int anValue);
int fifo_pop(void);
/* Header End */

// We should not store more then this many entries in our array
#define ABSOLUTE_MAX_ENTRIES 1024

// How many entries in our FIFO queue
int gnMaxEntries = ABSOLUTE_MAX_ENTRIES;
module_param(gnMaxEntries, int, 0);

// Semaphore to block prod/cons if necessary
struct semaphore goInputsSema;
struct semaphore goSlotsRemainingSema;
struct semaphore goMutex;

// Queue to store the values
int ganQueue[ABSOLUTE_MAX_ENTRIES] = {-1};
int gnQueueHead;
int gnQueueTail;
int gnQueueEntries;

/**
 * Check if the queue is empty
 * Returns 1 on empty, 0 on everything else
 */
int fifo_is_empty()
{
    if (gnQueueEntries == 0)
    {
        return 1;
    }

    return 0;
}

/**
 * Check if the queue is full
 * Return 1 on full, 0 on everything else
 */
int fifo_is_full()
{
    if (gnQueueEntries < gnMaxEntries)
    {
        return 0;
    }

    return 1;
}

/**
 * Push new element to FIFO queue.
 * Returns 1 on success, 0 on failure.
 */
int fifo_push(int anValue)
{
    if (! fifo_is_full())
    {
        // Insert the value
        ganQueue[gnQueueHead] = anValue;

        // Circular increment head
        if (gnQueueHead < gnMaxEntries)
        {
            gnQueueHead++;
        }
        else
        {
            gnQueueHead = 0;
        }

        // Total entries now one more
        gnQueueEntries++;

        // Insert successful
        return anValue;
    }

    // Insert failed
    return -1;
}

/**
 * Fetch value from FIFO queue.
 * Returns value or -1 (failure)
 */
int fifo_pop()
{
    int lnRetVal;

    if (! fifo_is_empty())
    {
        // Get the value
        lnRetVal = ganQueue[gnQueueTail];

        // Circular increment tail
        if (gnQueueTail < gnMaxEntries)
        {
            gnQueueTail++;
        }
        else
        {
            gnQueueTail = 0;
        }

        // Total entries now one less
        gnQueueEntries--;

        // Insert successful
        return lnRetVal;
    }

    // Insert failed
    return -1;
}

/**
 * If read() called for this driver, return next value from queue
 */
static ssize_t my_read(struct file *f, char __user *buffer, size_t length, loff_t *offset)
{
    int lnCopyToUser;
    int lnOutBuffer;

    // Ensure valid write location
    if (! access_ok(VERIFY_WRITE, buffer, length))
    {
        printk(KERN_ERR "numpipe: access_ok failed for my_read\n");
        return -EFAULT;
    }

    // If there are no entries in the queue, block until there is an entry
    if (down_interruptible(&goInputsSema))
    {
        printk(KERN_INFO "numpipe: down_interruptible failed for InputsSema in my_read\n");
        return -EFAULT;
    }

    // We successfully read at this point, so there is one more slot open for writing
    if (down_interruptible(&goMutex))
    {
        up(&goInputsSema); // Return the lock from before as it succeeded
        printk(KERN_INFO "numpipe: down_interruptible failed for Mutex in my_read\n");
        return -EFAULT;
    }

    lnOutBuffer = fifo_pop();
    
    up(&goMutex);
    up(&goSlotsRemainingSema);

    lnCopyToUser = copy_to_user(buffer, &lnOutBuffer, length);

    if (lnCopyToUser > 0)
    {
        printk(KERN_ERR "numpipe: copy_to_user failed for my_read %d\n", lnCopyToUser);
        return -EFAULT;
    }

    return length;
}

/**
 * If write() called for this driver, add value to back of queue
 */
static ssize_t my_write(struct file *f, const char __user *buffer, size_t length, loff_t *offset)
{
    int lnCopyFromUser = 0;
    int *lnValueFromUser = kzalloc(length, GFP_KERNEL);

    if (! lnValueFromUser)
    {
        printk(KERN_ERR "numpipe: kzalloc failed for my_write\n");
        return -EFAULT;
    }

    lnCopyFromUser = copy_from_user(lnValueFromUser, buffer, length);

    if (lnCopyFromUser > 0)
    {
        printk(KERN_ERR "numpipe: copy_from_user failed for my_write %d\n", lnCopyFromUser);
        return -EFAULT;
    }

    // If the queue is full, block until there is room
    if (down_interruptible(&goSlotsRemainingSema))
    {
        printk(KERN_INFO "numpipe: down_interruptible failed for SlotsRemaining in my_write\n");
        return -EFAULT;
    }

    // We successfully wrote at this point, so there is one more input in the queue
    if (down_interruptible(&goMutex))
    {
        up(&goSlotsRemainingSema); // Return the lock from before as it succeeded
        printk(KERN_INFO "numpipe: down_interruptible failed for Mutex in my_write\n");
        return -EFAULT;
    }
    
    fifo_push(*lnValueFromUser);
    up(&goMutex);
    up(&goInputsSema);

    return length;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

static struct miscdevice my_numpipe_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "numpipe",
    .fops = &my_fops
};

/**
 * Module initializer
 */
int __init init_module(void)
{
    // Register with OS (put into /dev/numpipe)
    misc_register(&my_numpipe_device);

    // Initialize queue counters
    gnQueueHead = 0;
    gnQueueTail = 0;
    gnQueueEntries = 0;

    // Initialize semaphores
    sema_init(&goInputsSema, 0); // FULL
    sema_init(&goSlotsRemainingSema, gnMaxEntries); // EMPTY
    sema_init(&goMutex, 1);

    // Alert we've fully loaded
    printk(KERN_INFO "numpipe: Loaded into Kernel w/ FIFO Size %d\n", gnMaxEntries);

    return 0;
}

/**
 * Module remover
 */
void __exit cleanup_module(void)
{
    printk(KERN_INFO "numpipe: Removed from Kernel\n");

    misc_deregister(&my_numpipe_device);
}
