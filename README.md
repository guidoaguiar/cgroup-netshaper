# cgroup-netshaper

A Linux kernel module that implements per-cgroup network bandwidth limits.

## Overview

cgroup-netshaper provides a new cgroup v2 controller called `net_shaper`, which allows direct bandwidth limitation for processes and containers without requiring complex tc/iptables configuration.

### Problem

Currently, limiting network bandwidth for Linux processes requires combining several components:
- cgroups with net_cls to mark packets
- tc (traffic control) for actual bandwidth shaping
- iptables/netfilter for more complex rules

This creates unnecessary complexity that could be handled directly in the kernel via cgroups.

### Solution

cgroup-netshaper implements a native kernel-level network bandwidth shaper that works directly with the cgroup v2 hierarchy, providing:

- Simple rate limiting per cgroup
- Direct integration with systemd and other cgroup v2 managers
- High-performance packet processing entirely in kernel space
- Easy configuration via the cgroup filesystem

## Features

- **Per-cgroup bandwidth limits**: Set input/output rate limits in bytes/second
- **Burst configuration**: Allow short-term bursts beyond the base rate
- **sysfs Interface**: Simple configuration through cgroup filesystem
- **systemd Integration**: Works naturally with systemd slices and services
- **Container Support**: Compatible with container runtimes using cgroup v2

## Usage

### Basic Configuration

```bash
# Create a systemd slice with network limits
systemd-run --slice=limited.slice -p MemoryMax=500M --unit=test.service /usr/bin/my-server

# Set network bandwidth limits (500 KB/s with 50 KB burst)
echo 500000 > /sys/fs/cgroup/limited.slice/cgroup.net_shaper.rate
echo 50000  > /sys/fs/cgroup/limited.slice/cgroup.net_shaper.burst

# Check current statistics
cat /sys/fs/cgroup/limited.slice/cgroup.net_shaper.stats
```

### Available Controls

Each cgroup directory provides the following files:

- `cgroup.net_shaper.rate`: Bandwidth limit in bytes per second
- `cgroup.net_shaper.burst`: Allowed burst size in bytes
- `cgroup.net_shaper.stats`: Current usage statistics

## Building and Installation

### Prerequisites

- Linux kernel headers (version 5.4+)
- Build tools (gcc, make)

### Compilation

```bash
git clone https://github.com/yourusername/cgroup-netshaper.git
cd cgroup-netshaper
make
```

### Installation

```bash
sudo insmod cgroup-netshaper.ko
```

## Documentation

For detailed documentation, see the [docs directory](./docs/).

## Development

Contributions are welcome! Please see [CONTRIBUTING.md](./CONTRIBUTING.md) for details.

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](./LICENSE) file for details.