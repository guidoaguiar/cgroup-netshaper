# Building cgroup-netshaper

This document provides detailed instructions for building the cgroup-netshaper kernel module from source.

## Prerequisites

### Required Packages

#### Debian/Ubuntu
```bash
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r) git make
```

#### Fedora/RHEL/CentOS
```bash
sudo dnf install kernel-devel kernel-headers gcc make git
```

#### Arch Linux
```bash
sudo pacman -S linux-headers base-devel git
```

### Kernel Requirements

The cgroup-netshaper module requires:
- Linux kernel 5.4 or later (recommended)
- cgroup v2 support enabled
- Netfilter support enabled

You can check your kernel version with:
```bash
uname -r
```

To verify cgroup v2 is enabled:
```bash
mount | grep cgroup2
```

If the command returns nothing, you may need to enable cgroup v2 in your boot parameters or mount it manually.

## Building the Module

1. Clone the repository (if you haven't already):
   ```bash
   git clone https://github.com/yourusername/cgroup-netshaper.git
   cd cgroup-netshaper
   ```

2. Build the module:
   ```bash
   make
   ```

   This will compile the module and create `cgroup_netshaper.ko`

3. Verify the build was successful:
   ```bash
   ls -la *.ko
   ```

## Build Options

You can customize the build with the following environment variables:

- `KERNELDIR`: Path to the kernel headers directory
  ```bash
  KERNELDIR=/usr/src/linux-headers-5.15.0 make
  ```

- `DEBUG=1`: Enable debug output
  ```bash
  DEBUG=1 make
  ```

## Cross-Compiling

To cross-compile for a different architecture:

1. Install the appropriate compiler and kernel headers, for example:
   ```bash
   sudo apt-get install gcc-aarch64-linux-gnu linux-headers-arm64
   ```

2. Build with the cross-compiler:
   ```bash
   ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- make
   ```

## Troubleshooting Build Issues

### Missing Kernel Headers

If you encounter errors about missing kernel headers:
```bash
sudo apt-get install linux-headers-$(uname -r)
```

### Version Mismatch

If you see "version magic" errors when loading the module:
- Ensure you're building against the headers that match your running kernel
- Try specifying the kernel directory explicitly:
  ```bash
  KERNELDIR=/lib/modules/$(uname -r)/build make
  ```

### Compilation Errors

If you encounter compilation errors:
1. Check that your kernel headers are properly installed
2. Verify that your kernel supports all the features used by the module
3. Check if there were recent kernel API changes that affect the module

## Development Build

For development and debugging:

```bash
# Enable debug output
make clean
DEBUG=1 make

# Additional static analysis with sparse
make C=2
```