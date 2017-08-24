# Thunderbolt(TM) user-space components

## License
These components are distributed under a BSD-style license. See COPYING for the
full license.


## Overview
Thunderbolt™ technology is a transformational high-speed, dual protocol
I/O that provides unmatched performance with up to 40Gbps bi-directional
transfer speeds. It provides flexibility and simplicity by supporting both
data (PCIe, USB3.1) and video (DisplayPort) on a single cable connection
that can daisy-chain up to six devices.


## Features
The user-space components implement device approval support:
1. Easier interaction with the kernel module for approving connected devices.
2. ACL for auto-approving devices white-listed by the user.


## tbtacl
tbtacl is intended to be triggered by udev (see the udev rules in tbtacl.rules).
It auto-approves devices that are found in ACL.


## tbtadm
tbtadm is a user-facing CLI tool. It provides operations for device approval,
handling the ACL and more.


## Supported OSes
- Ubuntu* 16.04 and 17.04


## Kernel/Daemon Compatibility
The user-space components operate in coordination with the upstream Thunderbolt
kernel driver (found in v4.13-rc1) to provide the Thunderbolt functionalities.
These components are NOT compatible with the old out-of-tree Thunderbolt kernel
module.


## Changelog
### v0.9
- First official release

### Eng. drop 2
- tbtadm: more commands added (devices, topology, acl)
- tbtadm: 'remove' accepts route-string, not only UUID
- tbtadm: 'remove-all' prints removed entry count
- tbtadm: future compatibility with xdomain changes
- tbtacl: use sh instead of bash
- tbtacl: improved error reporting (using write.py to get the actual errno)
- tbtadm, tbtacl, tbtacl.rules: improvement and bug fixes in SL2 support


## Known Issues
- tbtadm should use a helper + polkit for better permission handling
- install script is missing
- man page is missing
- error reporting can be improved


## Resolved Issues
### v0.9
- tbtacl: use C++ instead of Python for write action (GitHub issue #19)

### Eng. drop 2
- tbtacl: fixed SL2 handling
- tbtacl: don't assume errno(1) is installed
- tbtacl.rules: correctly handle change with authorized==2 (for SL2)
- tbtadm: correctly handle multi-controller systems
- tbtadm: 'approve-all' - do nothing if SL isn't 1 or 2
- tbtadm: 'approve-all' - add key on SL2
- tbtadm: removing non-existing ACL entry is just a warning, not an error
- tbtadm: File class reports errors more accurately for write() and read()


## Information
The source for this code:
- https://github.com/01org/thunderbolt-software-user-space/tree/upstream-driver

Mailing list:
- thunderbolt-software@lists.01.org
- Register at: https://lists.01.org/mailman/listinfo/thunderbolt-software
- Archives at: https://lists.01.org/pipermail/thunderbolt-software/

For additional information about Thunderbolt technology visit:
- https://01.org/thunderbolt-sw
- https://thunderbolttechnology.net/
