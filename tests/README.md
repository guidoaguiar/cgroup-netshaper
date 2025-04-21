# cgroup-netshaper Tests

This directory contains test scripts for the cgroup-netshaper kernel module.

## Available Tests

- `run_tests.sh`: Basic functionality test script that verifies:
  - Proper module loading
  - Creation of cgroup control files
  - Reading/writing rate and burst parameters
  - Basic statistics functionality

## Running Tests

### Basic Testing

To run the basic functionality tests:

```bash
# Make sure the script is executable
chmod +x run_tests.sh

# Run the tests
./run_tests.sh
```

### Continuous Integration

The tests are automatically run in the GitHub Actions CI pipeline for:
- Every pull request to the main branch
- Every push to the main branch

See the `.github/workflows` directory for CI configuration.

## Test Environment Requirements

For local testing, you'll need:

- Linux kernel 5.4+ with cgroup v2 support
- Root access (to load kernel modules and create cgroups)
- The module must be built before running tests (`make` in project root)

## Adding New Tests

When adding new tests:

1. Create a new script in this directory
2. Update this README.md with information about your test
3. Ensure your test returns a non-zero exit code on failure

## Manual Testing Guidelines

Some features can't be easily tested in an automated fashion. When manually testing:

1. Test bandwidth limiting with different rate/burst combinations
2. Test behavior under high load
3. Test with multiple concurrent processes in the same cgroup
4. Verify proper cleanup when removing the module

For performance testing, you can use tools like `iperf` or `netperf` to measure throughput.