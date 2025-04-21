#include "netshaper.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cgroup.h>
#include <linux/version.h>
#include <net/netfilter/nf_hook.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cgroup-netshaper Contributors");
MODULE_DESCRIPTION("Network bandwidth limiting for cgroups");
MODULE_VERSION(NETSHAPER_VERSION);

/* Module parameters */
static unsigned int default_rate = NETSHAPER_DEFAULT_RATE;
module_param(default_rate, uint, 0644);
MODULE_PARM_DESC(default_rate, "Default bandwidth rate in bytes/second (default: 1MB/s)");

static unsigned int default_burst = NETSHAPER_DEFAULT_BURST;
module_param(default_burst, uint, 0644);
MODULE_PARM_DESC(default_burst, "Default burst size in bytes (default: 128KB)");

static unsigned int check_interval = NETSHAPER_CHECK_INTERVAL_MS;
module_param(check_interval, uint, 0644);
MODULE_PARM_DESC(check_interval, "Packet release check interval in milliseconds (default: 100ms)");

/* Cgroup subsystem definition */
struct cgroup_subsys_state *netshaper_css_alloc(struct cgroup_subsys_state *parent_css);
void netshaper_css_free(struct cgroup_subsys_state *css);
int netshaper_css_online(struct cgroup_subsys_state *css);
void netshaper_css_offline(struct cgroup_subsys_state *css);

/* Helper to get our cgroup state from a css */
struct netshaper_cgroup *css_netshaper(struct cgroup_subsys_state *css)
{
    return css ? container_of(css, struct netshaper_cgroup, css) : NULL;
}

/* Cgroup controller callbacks */
static struct cftype netshaper_files[] = {
    {
        .name = "cgroup.net_shaper.rate",
        .read_u64 = netshaper_rate_read,
        .write_u64 = netshaper_rate_write,
    },
    {
        .name = "cgroup.net_shaper.burst",
        .read_u64 = netshaper_burst_read,
        .write_u64 = netshaper_burst_write,
    },
    {
        .name = "cgroup.net_shaper.stats",
        .seq_show = netshaper_stats_show,
    },
    {}
};

/* Define the cgroup subsystem structure */
struct cgroup_subsys net_shaper_cgrp_subsys = {
    .css_alloc = netshaper_css_alloc,
    .css_free = netshaper_css_free,
    .css_online = netshaper_css_online,
    .css_offline = netshaper_css_offline,
    .attach = NULL, /* No special attach needed */
    .legacy_cftypes = netshaper_files,
    .dfl_cftypes = netshaper_files,
    .name = "net_shaper",
};
EXPORT_SYMBOL_GPL(net_shaper_cgrp_subsys);

static int __init netshaper_init(void)
{
    int ret;
    
    pr_info("%s: initializing network bandwidth limiter for cgroups v2\n", NETSHAPER_NAME);
    
    /* Register netfilter hooks */
    ret = netshaper_register_netfilter();
    if (ret < 0) {
        pr_err("%s: failed to register netfilter hooks: %d\n", NETSHAPER_NAME, ret);
        return ret;
    }
    
    return 0;
}

static void __exit netshaper_exit(void)
{
    pr_info("%s: removing network bandwidth limiter for cgroups\n", NETSHAPER_NAME);
    
    /* Unregister netfilter hooks */
    netshaper_unregister_netfilter();
}

module_init(netshaper_init);
module_exit(netshaper_exit);