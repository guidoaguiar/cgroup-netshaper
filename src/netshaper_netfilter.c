/* SPDX-License-Identifier: GPL-2.0 */
#include "netshaper.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <linux/cgroup.h>

/* Netfilter hook operations */
static struct nf_hook_ops netshaper_nf_hooks[] = {
	{
		.hook = netshaper_nf_hook_in,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST,
	},
	{
		.hook = netshaper_nf_hook_out,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FIRST,
	},
};

/* Socket lookup compatibility wrapper for different kernel versions */
static inline struct sock *netshaper_sk_lookup(struct net *net, struct sk_buff *skb, struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	/* Modern kernels use nf_sk_lookup_slow_v4 function */
	return nf_sk_lookup_slow_v4(net, skb, dev);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	/* Intermediate kernels might have a different signature */
	return nf_sk_lookup_slow(net, skb, dev);
#else
	/* Older kernels might use ip_route_me_harder or other mechanisms */
	return NULL; /* Provide a fallback or implement specific logic */
#endif
}

/* Get the cgroup that owns a socket */
static struct netshaper_cgroup *get_sock_netshaper(struct sock *sk)
{
	struct cgroup_subsys_state *css;

	if (!sk)
		return NULL;

	/* Use proper method for getting cgroup based on kernel version */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
	css = sock_cgroup_ptr(&sk->sk_cgrp_data)->subsys[net_shaper_cgrp_subsys.id];
#else
	css = task_css(current, net_shaper_cgrp_subsys.id);
#endif

	return css ? css_netshaper(css) : NULL;
}

/* Queue a packet for delayed processing */
static unsigned int queue_packet(struct netshaper_cgroup *netshaper_cg,
				struct sk_buff *skb,
				struct net_device *dev,
				int (*okfn)(struct net_device *dev, struct sk_buff *skb))
{
	struct queued_skb *qskb;
	unsigned long flags;

	/* Allocate a container for the queued packet */
	qskb = kmalloc(sizeof(*qskb), GFP_ATOMIC);
	if (!qskb) {
		atomic64_inc(&netshaper_cg->dropped_packets);
		return NF_DROP;
	}

	/* Fill in the container */
	qskb->skb = skb;
	qskb->len = skb->len;
	qskb->dev = dev;
	qskb->okfn = okfn;
	qskb->timestamp = jiffies;

	/* Add it to our queue */
	spin_lock_irqsave(&netshaper_cg->lock, flags);
	list_add_tail(&qskb->list, &netshaper_cg->queue);
	atomic64_inc(&netshaper_cg->delayed_packets);
	spin_unlock_irqrestore(&netshaper_cg->lock, flags);

	return NF_STOLEN;
}

/* Outbound packet hook */
unsigned int netshaper_nf_hook_out(void *priv, struct sk_buff *skb,
				 const struct nf_hook_state *state)
{
	struct netshaper_cgroup *netshaper_cg;
	struct sock *sk;
	unsigned long bytes_this_interval;
	unsigned int budget;

	/* Get the socket and associated cgroup */
	sk = skb->sk;
	if (!sk)
		return NF_ACCEPT;

	netshaper_cg = get_sock_netshaper(sk);
	if (!netshaper_cg)
		return NF_ACCEPT;

	/* Increment byte counter */
	atomic64_add(skb->len, &netshaper_cg->bytes_tx);

	/* Calculate rate budget for this interval */
	budget = (netshaper_cg->rate * NETSHAPER_CHECK_INTERVAL_MS) / 1000;
	budget += netshaper_cg->burst;

	/* Check if we're over budget */
	bytes_this_interval = atomic64_read(&netshaper_cg->bytes_tx) % budget;
	if (bytes_this_interval + skb->len > budget) {
		/* Queue the packet for delayed processing */
		return queue_packet(netshaper_cg, skb, state->out, state->okfn);
	}

	return NF_ACCEPT;
}

/* Inbound packet hook */
unsigned int netshaper_nf_hook_in(void *priv, struct sk_buff *skb,
				const struct nf_hook_state *state)
{
	struct netshaper_cgroup *netshaper_cg;
	struct sock *sk;
	unsigned long bytes_this_interval;
	unsigned int budget;

	/* Try to get the socket and associated cgroup */
	sk = skb->sk;
	if (!sk) {
		/* For incoming packets, look up the socket using our compatibility wrapper */
		sk = netshaper_sk_lookup(dev_net(state->in), skb, state->in);
		if (!sk)
			return NF_ACCEPT;
	}

	netshaper_cg = get_sock_netshaper(sk);
	if (!netshaper_cg) {
		if (sk != skb->sk)
			sock_gen_put(sk);
		return NF_ACCEPT;
	}

	/* Increment byte counter */
	atomic64_add(skb->len, &netshaper_cg->bytes_rx);

	/* Calculate rate budget for this interval */
	budget = (netshaper_cg->rate * NETSHAPER_CHECK_INTERVAL_MS) / 1000;
	budget += netshaper_cg->burst;

	/* Check if we're over budget */
	bytes_this_interval = atomic64_read(&netshaper_cg->bytes_rx) % budget;
	if (bytes_this_interval + skb->len > budget) {
		/* Queue the packet for delayed processing */
		unsigned int ret = queue_packet(netshaper_cg, skb, state->in, state->okfn);

		if (sk != skb->sk)
			sock_gen_put(sk);

		return ret;
	}

	if (sk != skb->sk)
		sock_gen_put(sk);

	return NF_ACCEPT;
}

/* Register netfilter hooks */
int netshaper_register_netfilter(void)
{
	int ret;

	ret = nf_register_net_hooks(&init_net, netshaper_nf_hooks,
				   ARRAY_SIZE(netshaper_nf_hooks));
	if (ret < 0) {
		pr_err("cgroup_netshaper: failed to register netfilter hooks\n");
		return ret;
	}

	return 0;
}

/* Unregister netfilter hooks */
void netshaper_unregister_netfilter(void)
{
	nf_unregister_net_hooks(&init_net, netshaper_nf_hooks,
			       ARRAY_SIZE(netshaper_nf_hooks));
}