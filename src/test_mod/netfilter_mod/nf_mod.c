#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>
#include <linux/skbuff.h>

#include <linux/ip.h>	// ip_hdr()
#include <linux/udp.h>	// udp_hdr()
#include <linux/tcp.h>	// tcp_hdr()

#include <linux/string.h> // memset()

struct nf_hook_ops hook_ops;

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *ip = ip_hdr(skb);

	if(ip->protocol == 17) // if udp
	{
		struct udphdr *udp = udp_hdr(skb);			
		
		printk("Hooked UDP Packet");
		printk("src -> dst : %pI4:%d -> %pI4:%d",
			&ip->saddr, be16_to_cpu(udp->source),
			&ip->daddr, be16_to_cpu(udp->dest));
	}
	else if(ip->protocol == 6) // if tcp
	{
		struct tcphdr *tcp = tcp_hdr(skb);

		printk("Hooked TCP Packet");
		printk("src -> dst : %pI4:%d -> %pI4:%d",
			&ip->saddr, be16_to_cpu(tcp->source),
			&ip->daddr, be16_to_cpu(tcp->dest));
	}

	printk("\n");
	return NF_ACCEPT; // packet accept
}

static int __init init_(void)
{
	hook_ops.hook = hook_func;
	hook_ops.hooknum = NF_INET_POST_ROUTING;
	hook_ops.pf = NFPROTO_IPV4;

	if( !nf_register_net_hook(&init_net, &hook_ops))
		printk("netfilter register success\n");
	else
		printk("netfilter register fail\n");

	return 0;
}

static void __exit exit_(void)
{
	nf_unregister_net_hook(&init_net, &hook_ops);
}

module_init(init_);
module_exit(exit_);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("INSLAB_JIHUN");
