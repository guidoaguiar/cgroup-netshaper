#!/bin/bash
#
# Test script for cgroup-netshaper kernel module
#

set -e  # Exit on error

echo "Running cgroup-netshaper tests..."

# Check if the module is loaded
if ! lsmod | grep -q "cgroup_netshaper"; then
    echo "Module not loaded. Attempting to load..."
    insmod ../cgroup_netshaper.ko || { echo "Failed to load module"; exit 1; }
fi

echo "Module loaded successfully."

# Check if cgroup v2 is mounted
if ! mount | grep -q "cgroup2"; then
    echo "Cgroup v2 not mounted. Attempting to mount..."
    mount -t cgroup2 none /sys/fs/cgroup 2>/dev/null || \
        { echo "Failed to mount cgroup v2. Is it supported?"; exit 1; }
fi

echo "Cgroup v2 available."

# Create test cgroup
TEST_CGROUP="/sys/fs/cgroup/netshaper_test"
mkdir -p $TEST_CGROUP || { echo "Failed to create test cgroup"; exit 1; }

# Test 1: Check if control files exist
echo -n "Testing control files existence... "
if [ -f "$TEST_CGROUP/cgroup.net_shaper.rate" ] && \
   [ -f "$TEST_CGROUP/cgroup.net_shaper.burst" ] && \
   [ -f "$TEST_CGROUP/cgroup.net_shaper.stats" ]; then
    echo "OK"
else
    echo "FAILED"
    echo "Control files missing. Module not properly initialized."
    exit 1
fi

# Test 2: Check if we can set the rate
echo -n "Testing rate setting... "
echo "1048576" > "$TEST_CGROUP/cgroup.net_shaper.rate" || { echo "FAILED"; exit 1; }
RATE=$(cat "$TEST_CGROUP/cgroup.net_shaper.rate")
if [ "$RATE" == "1048576" ]; then
    echo "OK"
else
    echo "FAILED (set to $RATE instead of 1048576)"
    exit 1
fi

# Test 3: Check if we can set the burst
echo -n "Testing burst setting... "
echo "102400" > "$TEST_CGROUP/cgroup.net_shaper.burst" || { echo "FAILED"; exit 1; }
BURST=$(cat "$TEST_CGROUP/cgroup.net_shaper.burst")
if [ "$BURST" == "102400" ]; then
    echo "OK"
else
    echo "FAILED (set to $BURST instead of 102400)"
    exit 1
fi

# Test 4: Check if stats are readable
echo -n "Testing stats readability... "
if cat "$TEST_CGROUP/cgroup.net_shaper.stats" > /dev/null; then
    echo "OK"
else
    echo "FAILED"
    exit 1
fi

# Cleanup
echo -n "Cleaning up... "
rmdir $TEST_CGROUP || echo "Warning: Could not remove test cgroup"
echo "Done"

echo "All tests passed successfully!"
exit 0