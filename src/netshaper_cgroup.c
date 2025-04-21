/* SPDX-License-Identifier: GPL-2.0 */
#include "netshaper.h"

#include <linux/cgroup.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/jiffies.h>

/* Allocate a new cgroup state when a cgroup is created */
struct cgroup_subsys_state *netshaper_css_alloc(struct cgroup_subsys_state *parent_css)
{
	struct netshaper_cgroup *netshaper_cg;

	netshaper_cg = kzalloc(sizeof(*netshaper_cg), GFP_KERNEL);
	if (!netshaper_cg)
		return ERR_PTR(-ENOMEM);

	/* Initialize with default values */
	atomic64_set(&netshaper_cg->bytes_tx, 0);
	atomic64_set(&netshaper_cg->bytes_rx, 0);
	atomic64_set(&netshaper_cg->dropped_packets, 0);
	atomic64_set(&netshaper_cg->delayed_packets, 0);

	netshaper_cg->rate = parent_css ?
		css_netshaper(parent_css)->rate : NETSHAPER_DEFAULT_RATE;
	netshaper_cg->burst = parent_css ?
		css_netshaper(parent_css)->burst : NETSHAPER_DEFAULT_BURST;

	spin_lock_init(&netshaper_cg->lock);
	INIT_LIST_HEAD(&netshaper_cg->queue);
	INIT_DELAYED_WORK(&netshaper_cg->release_work, netshaper_release_queued_packets);

	return &netshaper_cg->css;
}

/* Called when a cgroup comes online */
int netshaper_css_online(struct cgroup_subsys_state *css)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	/* Schedule periodic work to release queued packets */
	schedule_delayed_work(&netshaper_cg->release_work,
			     msecs_to_jiffies(NETSHAPER_CHECK_INTERVAL_MS));

	return 0;
}

/* Called when a cgroup goes offline */
void netshaper_css_offline(struct cgroup_subsys_state *css)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);
	struct list_head *pos, *tmp;
	struct queued_skb *qskb;

	/* Cancel any pending work */
	cancel_delayed_work_sync(&netshaper_cg->release_work);

	/* Free any queued packets */
	spin_lock(&netshaper_cg->lock);
	list_for_each_safe(pos, tmp, &netshaper_cg->queue) {
		qskb = list_entry(pos, struct queued_skb, list);
		list_del(pos);
		kfree_skb(qskb->skb);
		kfree(qskb);
	}
	spin_unlock(&netshaper_cg->lock);
}

/* Free the cgroup state */
void netshaper_css_free(struct cgroup_subsys_state *css)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	kfree(netshaper_cg);
}

/* Release queued packets according to rate limits */
void netshaper_release_queued_packets(struct work_struct *work)
{
	struct netshaper_cgroup *netshaper_cg = container_of(to_delayed_work(work),
							    struct netshaper_cgroup,
							    release_work);
	struct list_head *pos, *tmp;
	struct queued_skb *qskb;
	unsigned long now = jiffies;
	unsigned int budget;
	unsigned long flags;

	/* Calculate how many bytes we can transmit in this interval */
	budget = (netshaper_cg->rate * NETSHAPER_CHECK_INTERVAL_MS) / 1000;
	budget += netshaper_cg->burst; /* Allow for bursting */

	/* Release packets up to our budget */
	spin_lock_irqsave(&netshaper_cg->lock, flags);
	list_for_each_safe(pos, tmp, &netshaper_cg->queue) {
		qskb = list_entry(pos, struct queued_skb, list);

		if (budget >= qskb->len) {
			budget -= qskb->len;
			list_del(pos);

			/* Re-inject the packet */
			spin_unlock_irqrestore(&netshaper_cg->lock, flags);
			
			/* Safety check for callback function */
			if (qskb->okfn && qskb->skb && qskb->dev) {
				qskb->okfn(qskb->dev, qskb->skb);
			} else {
				/* If callback is invalid, free the skb ourselves */
				if (qskb->skb)
					kfree_skb(qskb->skb);
			}
			
			spin_lock_irqsave(&netshaper_cg->lock, flags);

			atomic64_dec(&netshaper_cg->delayed_packets);
			kfree(qskb);
		} else {
			/* Out of budget, stop releasing */
			break;
		}
	}
	spin_unlock_irqrestore(&netshaper_cg->lock, flags);

	/* Reschedule ourselves */
	schedule_delayed_work(&netshaper_cg->release_work,
			     msecs_to_jiffies(NETSHAPER_CHECK_INTERVAL_MS));
}

/* Read/write handlers for cgroup files */
u64 netshaper_rate_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	return netshaper_cg->rate;
}

int netshaper_rate_write(struct cgroup_subsys_state *css, struct cftype *cft, u64 val)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	netshaper_cg->rate = val;
	return 0;
}

u64 netshaper_burst_read(struct cgroup_subsys_state *css, struct cftype *cft)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	return netshaper_cg->burst;
}

int netshaper_burst_write(struct cgroup_subsys_state *css, struct cftype *cft, u64 val)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(css);

	netshaper_cg->burst = val;
	return 0;
}

/* Show statistics in the stats file */
int netshaper_stats_show(struct seq_file *sf, void *v)
{
	struct netshaper_cgroup *netshaper_cg = css_netshaper(seq_css(sf));

	seq_printf(sf, "bytes_tx: %lld\n",
		  atomic64_read(&netshaper_cg->bytes_tx));
	seq_printf(sf, "bytes_rx: %lld\n",
		  atomic64_read(&netshaper_cg->bytes_rx));
	seq_printf(sf, "dropped_packets: %lld\n",
		  atomic64_read(&netshaper_cg->dropped_packets));
	seq_printf(sf, "delayed_packets: %lld\n",
		  atomic64_read(&netshaper_cg->delayed_packets));
	seq_printf(sf, "rate: %u bytes/s\n", netshaper_cg->rate);
	seq_printf(sf, "burst: %u bytes\n", netshaper_cg->burst);

	return 0;
}