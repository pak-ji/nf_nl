#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#define NL_USER 31

struct sock *nl_sk = NULL;

static void nl_recv_msg(struct sk_buff *skb)
{
	
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	
	char *msg = "Hello from kernel";
	
	int msg_size;
	int res;
	int pid;

	printk(KERN_INFO "Enter : %s\n", __FUNCTION__);

	msg_size = strlen(msg);

	nlh = nlmsg_hdr(skb);
	printk(KERN_INFO "Received Data : %s\n", (char*)nlmsg_data(nlh));
	pid = nlh->nlmsg_pid; // for sending message to application

	skb_out = nlmsg_new(msg_size, 0);

	if(!skb_out)
	{
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0; // not set mcast group
	strncpy(nlmsg_data(nlh), msg, msg_size);
	
	res = nlmsg_unicast(nl_sk, skb_out, pid);

	if(res < 0)
		printk(KERN_INFO "Error while sending back to user\n");
}




static int __init init_(void) 
{
	// above the kernel 3.6
	struct netlink_kernel_cfg cfg = {
		.input = nl_recv_msg
	};

	printk("Enter : %s\n", __FUNCTION__);

	nl_sk = netlink_kernel_create(&init_net, NL_USER, &cfg);
	if(!nl_sk)
	{
		printk(KERN_ALERT "Error creating NL-socket\n");
		return -10;
	}

	return 0;
}




static void __exit exit_(void)
{
	printk("Enter : %s\n", __FUNCTION__);

	netlink_kernel_release(nl_sk);
}

module_init(init_);
module_exit(exit_);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("INSLAB-JIHUN");
