# cgroup-netshaper Design

This document describes the design and implementation details of the cgroup-netshaper kernel module.

## Architecture Overview

cgroup-netshaper integrates with the Linux cgroup v2 subsystem to provide network bandwidth limiting at the cgroup level. The design consists of:

1. A cgroup v2 subsystem controller (`net_shaper`)
2. Netfilter hooks for monitoring and controlling packet flow
3. A rate limiting mechanism based on token bucket algorithm

## Component Details

### Cgroup Controller (netshaper_cgroup.c)

The cgroup controller maintains the state for each cgroup and provides the interface files:

- **cgroup.net_shaper.rate**: Controls the bandwidth rate limit in bytes/sec
- **cgroup.net_shaper.burst**: Controls the allowed burst size in bytes
- **cgroup.net_shaper.stats**: Shows current statistics

The controller implements standard cgroup callbacks:
- `netshaper_css_alloc`: Allocates state when a cgroup is created
- `netshaper_css_online`: Called when a cgroup comes online
- `netshaper_css_offline`: Called when a cgroup goes offline
- `netshaper_css_free`: Frees resources when a cgroup is destroyed

### Netfilter Integration (netshaper_netfilter.c)

The module registers netfilter hooks at `NF_INET_PRE_ROUTING` and `NF_INET_LOCAL_OUT` to intercept packets:

- **Inbound traffic**: The PRE_ROUTING hook counts and potentially delays inbound packets
- **Outbound traffic**: The LOCAL_OUT hook counts and potentially delays outbound packets

Each packet is associated with a cgroup based on the owning socket or process.

### Rate Limiting Algorithm

The module implements a token bucket algorithm:

1. Each cgroup has a rate limit (bytes/second) and burst size (bytes)
2. Packets are tracked and counted against the rate limit
3. If a packet would exceed the rate, it's queued for delayed transmission
4. A periodic worker function releases queued packets according to the rate limit

## Data Structures

### netshaper_cgroup

This structure extends the cgroup subsystem state with:

- Rate and burst limits
- Packet and byte counters
- A queue for delayed packets
- A lock to protect the queue

### queued_skb

This structure stores a delayed packet:

- The sk_buff itself
- The device and callback function
- Length and timestamp information

## Packet Flow

1. A packet enters the netfilter hook
2. The cgroup owning the packet is identified
3. Byte counters are updated
4. If the packet would exceed the rate limit:
   - It's placed in a queue
   - A delayed work item is scheduled
5. Otherwise, the packet continues normally
6. The periodic worker releases queued packets according to the rate limit

## Performance Considerations

- Lock contention is minimized by using spinlocks only when modifying the packet queue
- Atomic operations are used for counters to avoid locks
- Budget calculation is optimized to minimize per-packet overhead

## Future Extensions

Potential future enhancements:

1. IPv6 support
2. Per-device limits
3. Per-protocol (TCP/UDP) limits
4. Fair queuing across cgroups
5. Integration with container runtimes