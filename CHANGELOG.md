# Changelog

All notable changes to cgroup-netshaper will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial implementation of the cgroup v2 net_shaper controller
- Support for per-cgroup bandwidth rate limiting
- Netfilter hooks for packet monitoring and rate limiting
- Configurable rate and burst settings via cgroup filesystem
- Module parameters for default values
- Basic testing framework
- Documentation in man page format

## [0.1.0] - 2025-04-21

### Added
- Initial project structure
- Basic documentation
- Core module implementation with netfilter integration
- cgroup v2 controller integration
- GitHub CI/CD workflow setup