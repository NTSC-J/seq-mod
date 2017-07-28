#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

static int seq_init(void)
{
	printk(KERN_ALERT "seq: init\n");
	return 0;
}

static void seq_exit(void)
{
	printk(KERN_ALERT "seq: exit\n");
}

module_init(seq_init);
module_exit(seq_exit);

