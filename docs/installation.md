# Installation Guide

This guide will help you build and install the cgroup-netshaper kernel module on your Linux system.

## Prerequisites

- Linux kernel headers (version 5.4 or later recommended)
- Build tools (gcc, make)
- Root access for loading the kernel module

## Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/cgroup-netshaper.git
   cd cgroup-netshaper
   ```

2. Build the kernel module:
   ```bash
   make
   ```
   This will compile the module and create `cgroup_netshaper.ko`.

## Installation

1. Load the module:
   ```bash
   sudo insmod cgroup_netshaper.ko
   ```

2. Verify that the module was loaded:
   ```bash
   lsmod | grep cgroup_netshaper
   ```

3. Check kernel logs for any messages:
   ```bash
   dmesg | grep cgroup_netshaper
   ```

## Making the Installation Persistent

To make the module load automatically at boot:

1. Install the module to the system modules directory:
   ```bash
   sudo make install
   ```

2. Add the module to `/etc/modules`:
   ```bash
   echo cgroup_netshaper | sudo tee -a /etc/modules
   ```

## Uninstallation

1. Unload the module:
   ```bash
   sudo rmmod cgroup_netshaper
   ```

2. If you installed the module system-wide, remove it:
   ```bash
   sudo make uninstall
   ```

## Troubleshooting

### Module Loading Issues

If you encounter errors loading the module, check the kernel logs:
```bash
dmesg | tail
```

Common issues include:
- Kernel version incompatibility
- Missing dependencies
- Permission problems

### Cgroup Mount Issues

Make sure cgroup v2 is mounted:
```bash
mount | grep cgroup2
```

If not, you may need to mount it:
```bash
sudo mount -t cgroup2 none /sys/fs/cgroup
```

### Rate Limiting Not Working

Check if the module is properly registered with cgroup subsystems:
```bash
ls -la /sys/fs/cgroup/*/cgroup.net_shaper.*
```

You should see the control files available in your cgroup directories.