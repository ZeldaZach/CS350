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
MODULE_VERSION("0.0.1");

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
int gnMaxEntries = 0;
module_param(gnMaxEntries, int, 0);

// Semaphore to block prod/cons if necessary
struct semaphore *goInputsSema;
struct semaphore *goSlotsRemainingSema;

// Queue to store the values
int ganQueue[ABSOLUTE_MAX_ENTRIES] = {-1};
int gnQueue_head = 0;
int gnQueue_tail = 0;
int gnQueue_entries = 0;


int fifo_is_empty()
{
    if (gnQueue_entries == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int fifo_is_full()
{
    if (gnQueue_entries < gnMaxEntries)
    {
        return 0;
    }
    else
    {
        return 0;
    }
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
        ganQueue[gnQueue_head] = anValue;
        
        // Circular increment head
        if (gnQueue_head < gnMaxEntries)
        {
            gnQueue_head++;
        }
        else
        {
            gnQueue_head = 0;
        }

        // Total entries now one more
        gnQueue_entries++;

        // Insert successful
        return 1;
    }
    
    // Insert failed
    return 0;
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
        lnRetVal = ganQueue[gnQueue_tail];

        // Circular increment tail
        if (gnQueue_tail < gnMaxEntries)
        {
            gnQueue_tail++;
        }
        else
        {
            gnQueue_tail = 0;
        }

        // Total entries now one less
        gnQueue_entries--;

        return lnRetVal;
    }

    return -1;
}

static ssize_t my_read(struct file *f, char __user *buffer, size_t length, loff_t *offset)
{
    int lnCopyToUser;
    char *lspOutBuffer = kzalloc(length, GFP_KERNEL);

    // If there are no entries in the queue, block until there is an entry
    down_interruptible(goInputsSema);

    // We successfully read at this point, so there is one more slot open for writing
    sprintf(lspOutBuffer, "%d", fifo_pop());
    up(goSlotsRemainingSema);

    if (! access_ok(VERIFY_WRITE, buffer, length))
    {
        printk(KERN_ERR "numpipe: access_ok failed for my_read\n");
        return -EFAULT;
    }

    lnCopyToUser = copy_to_user(buffer, lspOutBuffer, strlen(lspOutBuffer)+1);

    kfree(lspOutBuffer);
    if (lnCopyToUser > 0)
    {
        printk(KERN_ERR "numpipe: copy_to_user failed for my_read %d\n", lnCopyToUser);
        return -EFAULT;
    }

    return lnCopyToUser;
}

static ssize_t my_write(struct file *f, const char __user *buffer, size_t length, loff_t *offset)
{
    int *lnValueFromUser = kzalloc(length, GFP_KERNEL);
    int lnCopyFromUser = copy_from_user(lnValueFromUser, buffer, length);

    if (lnCopyFromUser > 0)
    {
        printk(KERN_ERR "numpipe: copy_from_user failed for my_write %d\n", lnCopyFromUser);
        return -EFAULT;
    }

    // If the queue is full, block until there is room
    down_interruptible(goSlotsRemainingSema);

    // We successfully wrote at this point, so there is one more input in the queue
    fifo_push(*lnValueFromUser);
    up(goInputsSema);

    return 0;
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

// called when module is installed
int __init init_module(void)
{
    // Initialize semaphores
    sema_init(goSlotsRemainingSema, gnMaxEntries);
    sema_init(goInputsSema, 0);

    // Initialize queue counters
    gnQueue_head = 0;
    gnQueue_tail = 0;
    gnQueue_entries = 0;

    // Register with OS (put into /dev/numpipe)
    misc_register(&my_numpipe_device);

    // Alert we've fully loaded
    printk(KERN_INFO "numpipe: Loaded into Kernel\n");

    return 0;
}

// called when module is removed
void __exit cleanup_module(void)
{
    printk(KERN_INFO "numpipe: Removed from Kernel\n");

    misc_deregister(&my_numpipe_device);
}
