/***
 * @file	: nf_nl_mod.c
 * @author	: inslab_jihun
 * @date	: 2021-06-24
 *
 * @brief	: combined the Netfilter with Netlink \
 * 		  for the Packet Capture
 ***/ 



#include <linux/module.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/string.h>
#include <net/sock.h>

#define NL_USER 31



/* netlink function */
struct sock *nl_sk = NULL;
static void nl_recv_func(struct sk_buff*);

/* netfilter function */
struct nf_hook_ops hook_ops;
unsigned int nf_hook_func(void*, struct sk_buff*, const struct nf_hook_state*);

/* module init, exit function */
static int __init init_(void);
static void __exit exit_(void);

/* module registration */
module_init(init_);
module_exit(exit_);
























/**
 * @func	: nl_recv_func()
 * @brief	: receiving the Netlink message
 */
static void nl_recv_func(struct sk_buff *skb)
{
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;

	int res;

	static int pid;
	static bool init = false;


	/*----- (doing) only works when called from nf_hook_func() -----*/
	if(init)
	{
		skb_out = nlmsg_new(skb->len, 0); // skb->data_len when fragmentation
		if(!skb_out)
		{
			printk("[NF_NL] : Error nlmsg_new() for new skb\n");
			return;
		}

		nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, skb->len, 0);
		NETLINK_CB(skb_out).dst_group = 0; // not set mcast group
		
		memcpy(nlmsg_data(nlh), skb->data, skb->len);

		res = nlmsg_unicast(nl_sk, skb_out, pid);
		if(res < 0)
			printk("[NF_NL] : Error nlmsg_unicast() for pkt\n");
	}

	/*----- (only once) Initiate a connection with the application -----*/
	if(!init)
	{
		char *msg = "init";
		int msg_size;

		msg_size = strlen(msg);

		nlh = nlmsg_hdr(skb);
		pid = nlh->nlmsg_pid; // nlh->nlhmsg_pid = app process ID

		skb_out = nlmsg_new(msg_size, 0);
		if(!skb_out)
		{
			printk("[NF_NL] : Error nlmsg_new() for new skb\n");
			return;
		}

		nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
		NETLINK_CB(skb_out).dst_group = 0; // not set mcast group

		memcpy(nlmsg_data(nlh), msg, msg_size);
	
		res = nlmsg_unicast(nl_sk, skb_out, pid);
		if(res < 0)
			printk("[NF_NL] : Error nlmsg_unicast() for init\n");

		init = true;
	}
}


/**
 * @func	: nf_hook_func()
 * @brief	: hooking the packet of linux net stack
 */
unsigned int nf_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *ip = ip_hdr(skb);

	/* if udp */
	if(ip->protocol == 17)
	{
		struct udphdr *udp = udp_hdr(skb);

		/* target app port == 9999 */
		if(be16_to_cpu(udp->dest) == 9999)
		{
			nl_recv_func(skb);
			return NF_STOLEN;
		}
	}

	return NF_ACCEPT;	
}


/**
 * @func	: init_()
 * @brief	: register the Netfilter and Netlink
 */
static int __init init_(void) 
{
	/*----- netlink registration -----*/
	struct netlink_kernel_cfg cfg = { // above the kernel 3.6
		.input = nl_recv_func
	};

	nl_sk = netlink_kernel_create(&init_net, NL_USER, &cfg);
	if(nl_sk)
	{
		printk("[NF_NL] : Netlink register success\n");
	}
	else
	{
		printk("[NF_NL] : Netlink register fail\n");
		return -10;
	}


	/*----- netfilter registration -----*/
	hook_ops.hook = nf_hook_func;
	hook_ops.hooknum = NF_INET_LOCAL_IN; // only hooking the local pkt
	hook_ops.pf = NFPROTO_IPV4; // hooking target layer

	if( !nf_register_net_hook(&init_net, &hook_ops) )
	{
		printk("[NF_NL] : Netfilter register success\n");
	}
	else
	{
		printk("[NF_NL] : NEtfilter register fail\n");
		return -10;
	}

	return 0;
}


/**
 * @func	: exit_()
 * @brief	: unregister the Netfilter and Netlink
 */
static void __exit exit_(void)
{
	netlink_kernel_release(nl_sk);
	nf_unregister_net_hook(&init_net, &hook_ops);
}
















MODULE_LICENSE("GPL");
MODULE_AUTHOR("INSLAB-JIHUN");
