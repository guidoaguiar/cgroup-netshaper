# Usage Guide for cgroup-netshaper

This guide provides detailed instructions on how to use the cgroup-netshaper kernel module to limit network bandwidth for processes and containers.

## Basic Concepts

cgroup-netshaper works with cgroup v2 to provide per-cgroup network bandwidth limits. The module:

1. Creates control files in each cgroup directory
2. Monitors network traffic for processes in each cgroup
3. Enforces bandwidth limits according to the configured settings

## Control Files

After loading the module, you'll see these files in each cgroup directory:

- **cgroup.net_shaper.rate**: Sets/shows the maximum bandwidth rate (bytes/second)
- **cgroup.net_shaper.burst**: Sets/shows the allowed burst size (bytes)
- **cgroup.net_shaper.stats**: Displays statistics about the cgroup's network usage

## Setting Bandwidth Limits

### Basic Example

Here's a simple example of limiting a process to 1 MB/s:

```bash
# Create a new cgroup
mkdir -p /sys/fs/cgroup/limited

# Set limits
echo 1048576 > /sys/fs/cgroup/limited/cgroup.net_shaper.rate  # 1 MB/s
echo 104857 > /sys/fs/cgroup/limited/cgroup.net_shaper.burst  # ~100 KB burst

# Move a process into the cgroup
echo $PID > /sys/fs/cgroup/limited/cgroup.procs

# Run a new process in the cgroup
echo $$ > /sys/fs/cgroup/limited/cgroup.procs
exec wget http://example.com/largefile
```

### Integration with systemd

For systemd-based systems:

```bash
# Create a service with bandwidth limits
cat > /etc/systemd/system/limited-app.service << EOF
[Unit]
Description=Network Limited Application

[Service]
ExecStart=/usr/bin/my-network-intensive-app
CPUWeight=100
MemoryMax=500M

# Network shaper properties (applied by ExecStartPost)
ExecStartPost=/bin/sh -c 'echo 1048576 > /sys/fs/cgroup/system.slice/limited-app.service/cgroup.net_shaper.rate'
ExecStartPost=/bin/sh -c 'echo 104857 > /sys/fs/cgroup/system.slice/limited-app.service/cgroup.net_shaper.burst'

[Install]
WantedBy=multi-user.target
EOF

# Enable and start the service
systemctl daemon-reload
systemctl enable --now limited-app.service
```

### Docker/Container Integration

With Docker and cgroup v2:

```bash
# Start a container with network limits
docker run --name limited-container \
  --cgroup-parent=limited.slice \
  -d nginx

# Apply limits to the container
echo 1048576 > /sys/fs/cgroup/limited.slice/docker-$(docker inspect --format '{{.ID}}' limited-container).scope/cgroup.net_shaper.rate
echo 104857 > /sys/fs/cgroup/limited.slice/docker-$(docker inspect --format '{{.ID}}' limited-container).scope/cgroup.net_shaper.burst
```

## Monitoring

### Checking Current Limits

To check the current limits for a cgroup:

```bash
cat /sys/fs/cgroup/limited/cgroup.net_shaper.rate
cat /sys/fs/cgroup/limited/cgroup.net_shaper.burst
```

### Viewing Statistics

To view statistics about network usage:

```bash
cat /sys/fs/cgroup/limited/cgroup.net_shaper.stats
```

This will show:
- Total bytes transmitted and received
- Packets dropped due to rate limiting
- Packets delayed for later transmission
- Current rate and burst settings

## Advanced Usage

### Hierarchical Limits

cgroup-netshaper respects the hierarchical nature of cgroups:

- Child cgroups inherit limits from parents by default
- Each cgroup can override its own limits

Example:

```bash
# Set parent limit
echo 10485760 > /sys/fs/cgroup/parent/cgroup.net_shaper.rate  # 10 MB/s

# Create child with stricter limit
mkdir -p /sys/fs/cgroup/parent/child
echo 1048576 > /sys/fs/cgroup/parent/child/cgroup.net_shaper.rate  # 1 MB/s
```

### Testing Limits

You can verify the bandwidth limiting with standard tools:

```bash
# Move to the limited cgroup
echo $$ > /sys/fs/cgroup/limited/cgroup.procs

# Test download speed
wget http://speedtest.tele2.net/10MB.zip -O /dev/null

# Test upload speed
dd if=/dev/zero bs=1M count=100 | curl -s -T - http://dev/null
```

## Troubleshooting

### Module Not Working

If bandwidth limiting doesn't seem to work:

1. Verify the module is loaded:
   ```
   lsmod | grep cgroup_netshaper
   ```

2. Check that cgroup v2 is properly mounted:
   ```
   mount | grep cgroup2
   ```

3. Verify the control files exist:
   ```
   ls -la /sys/fs/cgroup/limited/cgroup.net_shaper.*
   ```

4. Check if any packets are being processed:
   ```
   cat /sys/fs/cgroup/limited/cgroup.net_shaper.stats
   ```

### Performance Issues

If you notice system performance problems:

1. Try increasing the burst size to allow more traffic spikes
2. Reduce the number of cgroups with active limiting
3. Check system logs for any error messages

## Performance Tuning

For best performance:

- Use reasonable rate limits (extremely low limits can cause excessive packet queuing)
- Set burst sizes appropriate for your application (typically 5-10% of the rate)
- Avoid creating too many rate-limited cgroups
- Consider the total traffic across all limited cgroups