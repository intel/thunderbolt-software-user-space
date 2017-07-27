Thunderbolt(TM) user-space components
*************************************

License
=======
These components are distributed under a BSD-style license. See COPYING for the
full license.


Overview
========
Thunderbolt™ technology is a transformational high-speed, dual protocol
I/O that provides unmatched performance with up to 40Gbps bi-directional
transfer speeds. It provides flexibility and simplicity by supporting both
data (PCIe, USB3.1) and video (DisplayPort) on a single cable connection
that can daisy-chain up to six devices.


Warning
=======
This is an engineering drop. The build definitions are still in preliminary
state, many options are still missing from the tbtadm tool and documentation is
not in its best state.


Features
========
The user-space components implement device approval support:
1. Easier interaction with the kernel module for approving connected devices.
2. ACL for auto-approving devices white-listed by the user.


tbtacl
======
tbtacl is intended to be triggered by udev (see the udev rules in tbtacl.rules).
It auto-approves devices that are found in ACL.


tbtadm
======
tbtadm is a user-facing CLI tool. It provides operations for device approval,
handling the ACL and more.


Supported OSes
==============
- Ubuntu* 16.04 and 17.04


Kernel/Daemon Compatibility
===========================
The user-space components operate in coordination with the upstream Thunderbolt
kernel driver (found in v4.13-rc1) to provide the Thunderbolt functionalities.
These components are NOT compatible with the old out-of-tree Thunderbolt kernel
module.


Known Issues
============


Resolved Issues
===============


Information
===========
The source for this code:
    https://github.com/01org/thunderbolt-software-user-space/tree/upstream-driver

Mailing list:
    thunderbolt-software@lists.01.org
    Register at: https://lists.01.org/mailman/listinfo/thunderbolt-software
    Archives at: https://lists.01.org/pipermail/thunderbolt-software/

For additional information about Thunderbolt technology visit:
    https://01.org/thunderbolt-sw
    https://thunderbolttechnology.net/
