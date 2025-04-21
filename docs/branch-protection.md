# Setting Up Branch Protection Rules

This document explains how to configure branch protection rules in GitHub to enforce code quality and testing requirements for the cgroup-netshaper project.

## Why Branch Protection?

Branch protection rules help maintain code quality by:
- Requiring tests to pass before merging
- Enforcing code review standards
- Preventing direct pushes to important branches
- Ensuring all code meets the project's quality standards

## Required Branch Protection Settings

### For Repository Administrators

1. Go to the GitHub repository page
2. Click on **Settings** (requires admin permissions)
3. In the left sidebar, click on **Branches**
4. Under "Branch protection rules", click **Add rule**

### Configure the following settings:

1. **Branch name pattern**: Enter `main` (or `master` if that's your default branch)

2. Check the following options:
   - ✅ **Require a pull request before merging**
     - ✅ Require approvals (recommended: at least 1)
     - ✅ Dismiss stale pull request approvals when new commits are pushed
   
   - ✅ **Require status checks to pass before merging**
     - ✅ Require branches to be up to date before merging
     - In the **Status checks that are required** field, search for and add:
       - `build-and-test` (Ensures kernel module builds properly)
       - `Code Quality Checks` (Ensures code follows kernel coding standards)
   
   - ✅ **Require conversation resolution before merging**
   
   - ✅ **Include administrators** (to ensure everyone follows the same rules)

3. Click **Create** to save the rule

## Verifying Branch Protection

After setting up branch protection:

1. Create a test branch with a simple change
2. Create a pull request
3. Verify that the status checks run automatically
4. Try to merge without the checks passing - it should be prevented
5. Fix any issues so the checks pass, then merge successfully

## Understanding CI/CD Failures

If CI/CD checks fail, click on the failing check for detailed information:

1. **Kernel Module Tests**: Indicates issues with the module compilation or basic structure
2. **Code Quality Checks**: Combines results from three different checks:
   - **checkpatch**: Validates code against Linux kernel style guidelines
   - **static-analysis**: Runs tools like sparse and cppcheck
   - **kernel-coding-style**: Verifies formatting matches kernel style

## Keeping Branch Protection Updated

As the project evolves:

1. Regularly review and update branch protection rules
2. Add new required checks as they're developed
3. Adjust settings based on team size and project needs

## Bypassing in Emergencies

In rare emergency situations, repository administrators can:

1. Temporarily disable branch protection
2. Make the emergency fix
3. Re-enable branch protection immediately after

This should be done only in critical situations and documented thoroughly.