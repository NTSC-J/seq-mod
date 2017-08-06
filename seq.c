/*
 * seq.c - Copyright 2017 Fuga Kato
 *
 * This file is part of seq-mod.
 *
 * seq-mod is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * seq-mod is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with seq-mod.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/limits.h>
#include "seq_ioctl.h"

#define SEQDEV_BUFSIZE PAGE_SIZE

static DEFINE_MUTEX(seqdev_mutex);

struct seqdev_data_t {
	int begin;
	int step;
	int end;
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
	struct seqdev_file_t *f;

	file->private_data = kzalloc(sizeof(struct seqdev_file_t), GFP_KERNEL);
	f = file->private_data;
	f->buf = kzalloc(SEQDEV_BUFSIZE, GFP_KERNEL);
	f->pos = seqdev_data.begin;

	return 0;
}

static int seqdev_release(struct inode *inode, struct file *file)
{
	struct seqdev_file_t *f;

	f = file->private_data;
	kfree(f->buf);
	kfree(f);
	f = NULL;

	return 0;
}

static ssize_t seqdev_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	/* TODO: countがけっこう大きい場合 */
	/* TODO: countが極端に小さい場合 */

	struct seqdev_file_t *f;
	static char linebuf[16];
	size_t bufsize, w;
	ssize_t read_size;

	f = file->private_data;
	bufsize = min(count, SEQDEV_BUFSIZE);
	w = 0;

	mutex_lock(&seqdev_mutex);
	for (;;) {
		size_t s;

		if (seqdev_data.end < f->pos)
			break;
		s = snprintf(linebuf, 16, "%d%c",
				    f->pos, seqdev_data.delimiter);
		if (w + s + 1 < bufsize) {
			memcpy(f->buf + w, linebuf, s);
			w += s;
			f->pos += seqdev_data.step;
		} else {
			break;
		}
	}
	mutex_unlock(&seqdev_mutex);

	read_size = w - copy_to_user(buf, f->buf, w);
	*ppos += read_size;

	return read_size;
}

static ssize_t seqdev_write(struct file *filp, const char __user *buf,
			    size_t count, loff_t *ppos)
{
	int arg1, arg2, arg3, argc;
	char *input;

	input = kmalloc(count, GFP_KERNEL);

	mutex_lock(&seqdev_mutex);

	if (copy_from_user(input, buf, count)) {
		mutex_unlock(&seqdev_mutex);
		return -1;
	}

	argc = sscanf(input, "%d%d%d", &arg1, &arg2, &arg3);

	switch (argc) {
	case 1:
		seqdev_data.begin = 1;
		seqdev_data.step = 1;
		seqdev_data.end = arg1;
		break;
	case 2:
		seqdev_data.begin = arg1;
		seqdev_data.step = 1;
		seqdev_data.end = arg2;
		break;
	case 3:
		seqdev_data.begin = arg1;
		seqdev_data.step = arg2;
		seqdev_data.end = arg3;
		break;
	default:
		mutex_unlock(&seqdev_mutex);
		return -1;
	}

	mutex_unlock(&seqdev_mutex);

	return count;
}

static long seqdev_ioctl(struct file *filp, unsigned int cmd,
			 unsigned long arg)
{
	char __user *cp = (char *)arg;
	int __user *ip = (int *)arg;

	switch (cmd) {
	case SEQ_IOCTL_SET_DELIMITER:
		return -copy_from_user(&seqdev_data.delimiter, cp, 1);
	case SEQ_IOCTL_GET_BEGIN:
		return -copy_to_user(ip, &seqdev_data.begin, sizeof(int));
	case SEQ_IOCTL_GET_END:
		return -copy_to_user(ip, &seqdev_data.end, sizeof(int));
	case SEQ_IOCTL_GET_STEP:
		return -copy_to_user(ip, &seqdev_data.step, sizeof(int));
	case SEQ_IOCTL_GET_DELIMITER:
		return -copy_to_user(cp, &seqdev_data.delimiter, 1);
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
		printk(KERN_ALERT "seq: misc_register returned %d\n", r);
		return r;
	}

	return 0;
}

static void __exit seqdev_exit(void)
{
	misc_deregister(&seqdev_dev);
}

MODULE_AUTHOR("Fuga Kato");
MODULE_LICENSE("GPL");
module_init(seqdev_init);
module_exit(seqdev_exit);

