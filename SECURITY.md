# Security Policy

## Supported Versions

The following versions of cgroup-netshaper are currently supported with security updates:

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |

## Reporting a Vulnerability

We take the security of cgroup-netshaper seriously. If you believe you have found a security vulnerability, please follow these steps:

1. **Do NOT disclose the vulnerability publicly**
2. **Email the maintainers directly** at security@example.com with:
   - A description of the vulnerability
   - Steps to reproduce the issue
   - Potential impact of the vulnerability
   - Any suggested mitigation or fix if available

### What to Expect

- **Acknowledgment**: We will acknowledge your report within 48 hours
- **Updates**: We will provide regular updates on our progress
- **Disclosure**: We will coordinate with you on the disclosure timeline
- **Credit**: We will acknowledge your responsible disclosure in our release notes unless you prefer to remain anonymous

## Security Best Practices

When using cgroup-netshaper:

1. **Keep the module updated** to the latest version
2. **Run with least privilege** - avoid running with unnecessary root privileges
3. **Control access** to the cgroup filesystem to prevent unauthorized changes to rate limits
4. **Monitor system logs** for unusual behavior related to the module

## Security Design Principles

The cgroup-netshaper module has been designed with the following security principles:

1. **Minimal kernel footprint** to reduce potential attack surface
2. **Input validation** on all configurable parameters
3. **Memory safety** through careful allocation and deallocation
4. **Protection against overflow** in counter variables and calculations

## Security Considerations

Please be aware that kernel modules operate with high privileges. Users should:

- Only install this module on systems where you trust all users with root access
- Be aware that any security vulnerability in a kernel module can potentially affect the entire system
- Apply regular kernel security updates