# Contributing to cgroup-netshaper

Thank you for your interest in contributing to cgroup-netshaper! This document provides guidelines and instructions for contributing to this project.

## Code of Conduct

This project adheres to a Code of Conduct that all participants are expected to follow. Please read and understand it before contributing.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Set up the development environment** (see below)
4. **Create a branch** for your changes
5. **Make your changes**
6. **Submit a pull request**

## Development Environment Setup

Ensure you have:
- Linux kernel headers (5.4+)
- GCC and Make
- A system with cgroup v2 support for testing

To set up your development environment:

```bash
# Install dependencies on Debian/Ubuntu
sudo apt-get install build-essential linux-headers-$(uname -r)

# Clone your fork
git clone https://github.com/YOUR_USERNAME/cgroup-netshaper.git
cd cgroup-netshaper

# Add the upstream repository as a remote
git remote add upstream https://github.com/ORIGINAL_OWNER/cgroup-netshaper.git
```

## Making Changes

1. Always create a new branch for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. Follow the kernel coding style (see [Linux kernel coding style](https://www.kernel.org/doc/html/latest/process/coding-style.html))

3. Include appropriate comments and documentation

4. Write meaningful commit messages that explain your changes

## Testing

1. Compile the module:
   ```bash
   make
   ```

2. Test your changes (basic tests):
   ```bash
   cd tests
   ./run_tests.sh
   ```

3. Install and test manually:
   ```bash
   sudo insmod cgroup_netshaper.ko
   # Test using the provided examples in the docs directory
   ```

## Submitting Changes

1. Push your changes to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

2. Create a pull request on GitHub with a clear description of:
   - What the change does
   - Why it's needed
   - How it's implemented
   - Any testing you've performed

3. Address any feedback from reviewers

## Code Style Guidelines

- Follow the Linux kernel coding style
- Use 8-space indentation (tabs, not spaces)
- Keep lines under 80 characters when possible
- Use K&R brace style
- Add meaningful comments
- Avoid unnecessary whitespace changes

## Documentation

If your change adds new functionality or changes existing behavior, update:
- Code comments
- Relevant documentation files in the `docs/` directory
- The README.md file if necessary

## License

By contributing to this project, you agree that your contributions will be licensed under the project's GNU GPL v2 license.