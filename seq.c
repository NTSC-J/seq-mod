#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/limits.h>

struct seqdev_data_t {
	int begin;
	int end;
	int step;
};
static struct seqdev_data_t seqdev_data = {
	.begin	= 1,
	.end	= INT_MAX,
	.step	= 1
};

/* seq_operations */
static void *seqdev_seq_start(struct seq_file *s, loff_t *pos)
{
	int *it = kmalloc(sizeof(int), GFP_KERNEL);
	if (!it)
		return NULL;
	*it = seqdev_data.begin;

	return it;
}

static void *seqdev_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	*((int *)v) += seqdev_data.step;
	return (*((int *)v) <= seqdev_data.end) ? v : NULL;
}

static void seqdev_seq_stop(struct seq_file *s, void *v)
{
	kfree(v);
}

static int seqdev_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "%d\n", *((int *)v));
	return 0;
}

static const struct seq_operations seqdev_seq_ops = {
	.start	= seqdev_seq_start,
	.next	= seqdev_seq_next,
	.stop	= seqdev_seq_stop,
	.show	= seqdev_seq_show
};

/* file_operations */
static int seqdev_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &seqdev_seq_ops);
}

static ssize_t seqdev_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	return count;
}

static long seqdev_ioctl(struct file *filp, unsigned int cmd,
		     unsigned long arg)
{
	return  -ENOTTY;
}

static struct file_operations seqdev_fops = {
	.owner		= THIS_MODULE,
	.open		= seqdev_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
	.write		= seqdev_write,
	.unlocked_ioctl	= seqdev_ioctl
};

static struct miscdevice seqdev_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "seq",
	.fops	= &seqdev_fops
};

static int __init seqdev_init(void)
{
	int r;
	r = misc_register(&seqdev_dev);
	if (r) {
		printk("seqdev: misc_register returned %d\n", r);
		return r;
	}

	printk("seqdev: loaded\n");
	return 0;
}

static void __exit seqdev_exit(void)
{
	printk(KERN_INFO "seqdev: exit\n");
	misc_deregister(&seqdev_dev);
}

MODULE_LICENSE("GPL");
module_init(seqdev_init);
module_exit(seqdev_exit);

