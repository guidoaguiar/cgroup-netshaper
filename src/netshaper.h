/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _NETSHAPER_H
#define _NETSHAPER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cgroup.h>
#include <linux/slab.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <net/sock.h>
#include <linux/init.h>

/* Compatibility macros for different kernel versions */
#ifndef HAS_NET_HOOKS
  #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
    #define nf_register_net_hooks(net, ops, n) nf_register_hooks(ops, n)
    #define nf_unregister_net_hooks(net, ops, n) nf_unregister_hooks(ops, n)
  #endif
#endif

#ifndef HAS_SOCK_GEN_PUT
  #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)
    #define sock_gen_put(sk) sock_put(sk)
  #endif
#endif

/* Module Information */
#define NETSHAPER_VERSION "0.1.0"
#define NETSHAPER_NAME "cgroup_netshaper"

/* Rate Limiting Constants */
#define NETSHAPER_DEFAULT_RATE (1024 * 1024)    /* 1 MB/s */
#define NETSHAPER_DEFAULT_BURST (128 * 1024)    /* 128 KB */
#define NETSHAPER_CHECK_INTERVAL_MS 100         /* 100ms check interval */

/* Cgroup file names */
#define NETSHAPER_RATE_FILENAME "cgroup.net_shaper.rate"
#define NETSHAPER_BURST_FILENAME "cgroup.net_shaper.burst"
#define NETSHAPER_STATS_FILENAME "cgroup.net_shaper.stats"

/* Netshaper cgroup state */
struct netshaper_cgroup {
	struct cgroup_subsys_state css;
	atomic64_t bytes_tx;             /* Bytes transmitted */
	atomic64_t bytes_rx;             /* Bytes received */
	atomic64_t dropped_packets;      /* Packets dropped */
	atomic64_t delayed_packets;      /* Packets delayed */

	unsigned int rate;               /* Bytes per second limit */
	unsigned int burst;              /* Burst size in bytes */

	spinlock_t lock;                 /* Lock for the packet queue */
	struct list_head queue;          /* Queue for delayed packets */
	struct delayed_work release_work; /* Work for releasing packets */
};

/* Packet container for delayed processing */
struct queued_skb {
	struct list_head list;
	struct sk_buff *skb;
	unsigned int len;
	struct net_device *dev;
	int (*okfn)(struct net_device *dev, struct sk_buff *skb);
	unsigned long timestamp;
};

/* Netshaper cgroup operations */
struct cgroup_subsys_state *netshaper_css_alloc(struct cgroup_subsys_state *parent_css);
int netshaper_css_online(struct cgroup_subsys_state *css);
void netshaper_css_offline(struct cgroup_subsys_state *css);
void netshaper_css_free(struct cgroup_subsys_state *css);

/* Netfilter hooks */
unsigned int netshaper_nf_hook_in(void *priv, struct sk_buff *skb,
				 const struct nf_hook_state *state);
unsigned int netshaper_nf_hook_out(void *priv, struct sk_buff *skb,
				  const struct nf_hook_state *state);
int netshaper_register_netfilter(void);
void netshaper_unregister_netfilter(void);

/* Helper functions */
struct netshaper_cgroup *css_netshaper(struct cgroup_subsys_state *css);
void netshaper_release_queued_packets(struct work_struct *work);

#endif /* _NETSHAPER_H */