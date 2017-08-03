#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/limits.h>

#define SEQDEV_BUFSIZE PAGE_SIZE

static DEFINE_MUTEX(seqdev_mutex);

struct seqdev_data_t {
	int begin;
	int end;
	int step;
	char delimiter;
};
static struct seqdev_data_t seqdev_data = {
	.begin		= 1,
	.end		= INT_MAX,
	.step		= 1,
	.delimiter	= '\n'
};
struct seqdev_file_t {
	int pos;
	char *buf;
};

/* file_operations */
static int seqdev_open(struct inode *inode, struct file *file)
{
	file->private_data = kzalloc(sizeof(struct seqdev_file_t), GFP_KERNEL);
	struct seqdev_file_t *f = file->private_data;
	f->buf = kzalloc(SEQDEV_BUFSIZE, GFP_KERNEL);
	f->pos = seqdev_data.begin;
	printk(KERN_INFO "seq: allocated buffers\n");
	return 0;
}

static int seqdev_release(struct inode *inode, struct file *file)
{
	struct seqdev_file_t *f = file->private_data;
	kfree(f->buf);
	kfree(f);
	printk(KERN_INFO "seq: freed buffers\n");
	return 0;
}

static ssize_t seqdev_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct seqdev_file_t *f = file->private_data;

	/* 行ごとにバッファ */
	static char linebuf[16];
	/* 実際に使うバッファサイズ */
	/* TODO: countがけっこう大きい場合 */
	/* TODO: countが極端に小さい場合 */
	size_t bufsize = min(count, SEQDEV_BUFSIZE);

	size_t w = 0;
	for (;;) {
		if (seqdev_data.end < f->pos)
			break;
		size_t s = snprintf(linebuf, 16, "%d%c",
				    f->pos, seqdev_data.delimiter);
		if (w + s + 1 < bufsize) {
			memcpy(f->buf + w, linebuf, s);
			w += s;
			f->pos += seqdev_data.step;
		} else {
			break;
		}
	}

	mutex_lock(&seqdev_mutex);
	ssize_t read_size = w - copy_to_user(buf, f->buf, w);
	mutex_unlock(&seqdev_mutex);
	*ppos += read_size;
	return read_size;
}

static ssize_t seqdev_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	int arg1, arg2, arg3, argc;
	char *input = kmalloc(count, GFP_KERNEL);
	if (copy_from_user(input, buf, count))
		return -1;

	argc = sscanf(input, "%d%d%d", &arg1, &arg2, &arg3);

	switch (argc) {
	case 1:
		seqdev_data.begin = 1;
		seqdev_data.end = arg1;
		seqdev_data.step = 1;
		break;
	case 2:
		seqdev_data.begin = arg1;
		seqdev_data.end = arg2;
		seqdev_data.step = 1;
		break;
	case 3:
		seqdev_data.begin = arg1;
		seqdev_data.end = arg2;
		seqdev_data.step = arg3;
		break;
	default:
		return -1;
	}
	return count;
}

#define SEQDEV_IOCTL_GROUP 'y'
#define SEQDEV_IOCTL_SET_DELIMITER _IOW(SEQDEV_IOCTL_GROUP, 1, char)
#define SEQDEV_IOCTL_GET_BEGIN _IOR(SEQDEV_IOCTL_GROUP, 2, int)
#define SEQDEV_IOCTL_GET_END _IOR(SEQDEV_IOCTL_GROUP, 3, int)
#define SEQDEV_IOCTL_GET_STEP _IOR(SEQDEV_IOCTL_GROUP, 4, int)
#define SEQDEV_IOCTL_GET_DELIMITER _IOR(SEQDEV_IOCTL_GROUP, 5, char)

static long seqdev_ioctl(struct file *filp, unsigned int cmd,
			 unsigned long arg)
{
	char *cp = (char *)arg;
	int *ip = (int *)arg;

	switch (cmd) {
	case SEQDEV_IOCTL_SET_DELIMITER:
		seqdev_data.delimiter = *cp;
		return 0;
	case SEQDEV_IOCTL_GET_BEGIN:
		*ip = seqdev_data.begin;
		return 0;
	case SEQDEV_IOCTL_GET_END:
		*ip = seqdev_data.end;
		return 0;
	case SEQDEV_IOCTL_GET_STEP:
		*ip = seqdev_data.step;
		return 0;
	case SEQDEV_IOCTL_GET_DELIMITER:
		*cp = seqdev_data.delimiter;
		return 0;
	default:
		return  -ENOTTY;
	}
}

static struct file_operations seqdev_fops = {
	.owner		= THIS_MODULE,
	.open		= seqdev_open,
	.release	= seqdev_release,
	.read		= seqdev_read,
	.write		= seqdev_write,
	.unlocked_ioctl	= seqdev_ioctl
};

static struct miscdevice seqdev_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "seq",
	.mode	= 0666,
	.fops	= &seqdev_fops
};

static int __init seqdev_init(void)
{
	int r;
	r = misc_register(&seqdev_dev);
	if (r) {
		printk("seq: misc_register returned %d\n", r);
		return r;
	}

	printk(KERN_INFO "seq: loaded\n");
	return 0;
}

static void __exit seqdev_exit(void)
{
	printk(KERN_INFO "seq: exit\n");
	misc_deregister(&seqdev_dev);
}

MODULE_LICENSE("GPL");
module_init(seqdev_init);
module_exit(seqdev_exit);

