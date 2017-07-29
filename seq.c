#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static ssize_t seq_read(struct file *filp, char __user *buf,
			size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t seq_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	return count;
}

static long seq_ioctl(struct file *filp, unsigned int cmd,
		     unsigned long arg)
{
	return  -ENOTTY;
}

static struct file_operations seq_fops = {
	.owner		= THIS_MODULE,
	.read		= seq_read,
	.write		= seq_write,
	.unlocked_ioctl	= seq_ioctl,
};

static struct miscdevice seq_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "seq",
	.fops	= &seq_fops
};

static int __init seq_init(void)
{
	int r;

	if ((r = misc_register(&seq_dev))) {
		printk("seq: misc_register failed\n");
		return r;
	}

	printk("seq: loaded\n");
	return 0;
}

static void __exit seq_exit(void)
{
	printk(KERN_INFO "seq: exit\n");
	misc_deregister(&seq_dev);
}

MODULE_LICENSE("GPL");
module_init(seq_init);
module_exit(seq_exit);

