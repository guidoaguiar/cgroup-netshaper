# CI/CD Setup Guide

This document explains how to configure the continuous integration (CI) and branch protection rules for the cgroup-netshaper project.

## Workflow Overview

The cgroup-netshaper project uses GitHub Actions for:

1. **Kernel Module Tests**: Build and test the kernel module on multiple kernel versions
2. **Code Quality Checks**: Ensure code follows Linux kernel style guidelines
   - Checkpatch verification
   - Static analysis (sparse, cppcheck)
   - Kernel coding style verification

## Blocking PR Merges on Test Failures

To ensure that code quality is maintained, the repository should be configured to block merges of pull requests that fail tests.

### Setting Up Branch Protection Rules

As a repository administrator, follow these steps:

1. Go to your GitHub repository → Settings → Branches
2. Under "Branch protection rules", click "Add rule"
3. In "Branch name pattern", enter `main` (or `master` if that's your default branch)
4. Enable the following options:
   - ✅ Require a pull request before merging
   - ✅ Require status checks to pass before merging
   - ✅ Require branches to be up to date before merging

5. In the "Status checks that are required" search field, add the following:
   - `build-and-test` (Kernel module build & test)
   - `Code Quality Checks` (Combined code quality results)

6. Optionally, you may also want to enable:
   - ✅ Require conversation resolution before merging
   - ✅ Require linear history
   - ✅ Include administrators

7. Click "Create" to save the rule

## Status Checks Explained

### Kernel Module Tests

This workflow validates that the kernel module:
- Builds successfully against multiple kernel versions
- Passes basic structure validation
- Passes static analysis with sparse

If any kernel version build fails, the PR will be blocked from merging.

### Code Quality Checks

These checks ensure that all code follows Linux kernel development standards:
- **checkpatch**: Validates style issues using the Linux kernel's official checkpatch.pl tool
- **static-analysis**: Runs deep static analysis tools (sparse, cppcheck)
- **kernel-coding-style**: Verifies code formatting matches kernel style

The combined status of all code quality checks is reported as a single status check called "Code Quality Checks", which is what you should add to the required checks in branch protection.

## Manual Testing

While the CI provides automated tests, some aspects of kernel module functionality cannot be fully tested in a CI environment. Contributors should also:

1. Test the module on real hardware when making significant changes
2. Follow the testing guide in `tests/README.md` for manual verification

## Troubleshooting CI Failures

If a CI check fails:

1. Click on the failing check in the PR to see detailed logs
2. Address the specific issues mentioned in the logs
3. Push new commits to the branch
4. The CI will automatically run again on the updated code

For persistent issues, check the [GitHub Actions documentation](https://docs.github.com/en/actions) or file an issue in the repository.