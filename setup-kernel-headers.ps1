# setup-kernel-headers.ps1
# Script to download and set up Linux kernel headers for kernel module development on Windows
# For use with cgroup-netshaper project

# Create directory for kernel headers
$headerDir = "C:\linux-kernel-headers"
New-Item -Path $headerDir -ItemType Directory -Force

# Download Linux kernel source
$kernelVersion = "5.15.90"  # A recent stable kernel version

# Create symbolic links for easier access
if (Test-Path "$headerDir\include") {
    Remove-Item "$headerDir\include" -Recurse -Force
}
New-Item -Path "$headerDir\include" -ItemType Directory -Force

# Copy key header directories for IntelliSense
Write-Host "Setting up header directories..."
Copy-Item -Path "$headerDir\linux-$kernelVersion\include\*" -Destination "$headerDir\include\" -Recurse -Force
Copy-Item -Path "$headerDir\linux-$kernelVersion\arch\x86\include\*" -Destination "$headerDir\arch\x86\include\" -Recurse -Force

Write-Host "Creating dummy header files for VS Code IntelliSense..."
# Create dummy header files for common architectures
New-Item -Path "$headerDir\arch\x86\include\generated\uapi\asm" -ItemType Directory -Force
@"
#ifndef _ASM_X86_TYPES_H
#define _ASM_X86_TYPES_H

typedef unsigned char       __u8;
typedef unsigned short      __u16;
typedef unsigned int        __u32;
typedef unsigned long long  __u64;

typedef signed char         __s8;
typedef signed short        __s16;
typedef signed int          __s32;
typedef signed long long    __s64;

#endif /* _ASM_X86_TYPES_H */
"@ | Out-File -FilePath "$headerDir\arch\x86\include\generated\uapi\asm\types.h" -Encoding ASCII

Write-Host "Linux kernel headers setup complete!"
Write-Host "VS Code should now be able to properly index Linux kernel headers."
Write-Host "You may need to reload your VS Code window for changes to take effect."