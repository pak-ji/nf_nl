#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("INSLAB_JIHUN");
MODULE_DESCRIPTION("A Test Simple Module");

static int __init init_(void){
	printk("Init, World\n");
	return 0;
}

static void __exit exit_(void)
{
	printk("Exit, World\n");
}

module_init(init_);
module_exit(exit_);
