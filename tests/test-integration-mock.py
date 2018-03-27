#!/usr/bin/python3
#
# thunderbolt-tools intergation tests. Modified version of
# bolt integration test suite
#
# Copyright © 2017 Red Hat, Inc
# Copyright © 2017 Intel Corp
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Authors:
#       Christian J. Kellner <christian@kellner.me>
#       Andrei Emeltchenko <andrei.emeltchenko@intel.com>

import binascii
import os
import shutil
import sys
import subprocess
import unittest
import uuid
import tempfile

import re

import logging
log = logging.getLogger(__name__)

import shlex

from itertools import chain

try:
    import gi
    from gi.repository import GLib
    from gi.repository import Gio
    gi.require_version('UMockdev', '1.0')
    from gi.repository import UMockdev

except ImportError as e:
    sys.stderr.write('Skipping integration test due to missing depdendencies: %s\n' % str(e))
    sys.exit(0)

# Configuration
TBTADM = "tbtadm/tbtadm"
ACL = "/var/lib/thunderbolt/acl"
SYSFS = "/sys/bus/thunderbolt"
VENDOR = "Mock Vendor"
DEVICE_NAME = "Thunderbolt Cable"

# Mock Device Tree
class Device(object):
    subsystem = "unknown"
    udev_attrs = []
    udev_props = []

    def __init__(self, name, children):
        self._parent = None
        self.children = [self._adopt(c) for c in children]
        self.udev = None
        self.name = name
        self.syspath = None

    def _adopt(self, device):
        device.parent = self
        return device

    def _get_own(self, items):
        i = chain.from_iterable([a, str(getattr(self, a.lower()))] for a in items)
        return list(i)

    def collect(self, predicate):
        children = self.children
        head = [self] if predicate(self) else []
        tail = chain.from_iterable(c.collect(predicate) for c in children)
        return head + list(filter(predicate, tail))

    def first(self, predicate):
        if predicate(self):
            return self
        for c in self.children:
            found = c.first(predicate)
            if found:
                return found

    @property
    def parent(self):
        return self._parent

    @parent.setter
    def parent(self, value):
        self._parent = value

    @property
    def root(self):
        return self if self.parent is None else self.parent.root

    def connect_tree(self, bed):
        self.connect(bed)
        for c in self.children:
            c.connect_tree(bed)

    def connect(self, bed):
        assert self.syspath is None
        attributes = self._get_own(self.udev_attrs)
        properties = self._get_own(self.udev_props)
        sysparent = self.parent and self.parent.syspath
        self.syspath = bed.add_device(self.subsystem,
                                      self.name,
                                      sysparent,
                                      attributes,
                                      properties)
        self.testbed = bed
        print('Connected ' + self.name + ' ' + self.syspath)

    def disconnect(self, bed):
        for c in self.children:
            c.disconnect(bed)
        print('disconnecting ' + self.name + ' ' + self.syspath)
        bed.remove_device(self.syspath)
        self.authorized = 0
        self.key = ""
        self.syspath = None

# Thunderbolt device class
class TbDevice(Device):
    subsystem = "thunderbolt"
    devtype = "thunderbolt_device"

    udev_attrs = ['authorized',
                  'device',
                  'device_name',
                  'key',
                  'unique_id',
                  'vendor',
                  'vendor_name']

    udev_props = ['DEVTYPE']

    def __init__(self, name, device_name=None, authorized=0, vendor=None,
                 uid=None, children=None):
        super(TbDevice, self).__init__(name, children or [])
        self.unique_id = uid or str(uuid.uuid4())
        self.device_name = device_name or 'Thunderbolt ' + name
        self.device = self._make_id(self.device_name)
        self.vendor_name = vendor or 'Mock Device'
        self.vendor = self._make_id(self.vendor_name)
        self.authorized = authorized
        self.key = ""

    def _make_id(self, name):
        return '0x%X' % binascii.crc32(name.encode('utf-8'))

    @property
    def authorized_file(self):
        if self.syspath is None:
            return None
        return os.path.join(self.syspath, 'authorized')

    @property
    def domain(self):
        return self.parent.domain

    @staticmethod
    def is_unauthorized(d):
        return isinstance(d, TbDevice) and d.authorized == 0

class TbHost(TbDevice):
    def __init__(self, children, index = 0):
        super(TbHost, self).__init__('%d-0' % index,
                                     authorized=1,
                                     uid='3b7d4bad-4fdf-44ff-8730-ffffdeadbabe',
                                     device_name='Controller',
                                     children=children)

    def connect(self, bed):
        self.authorized = 1
        super(TbHost, self).connect(bed)

class TbDomain(Device):
    subsystem = "thunderbolt"
    devtype = "thunderbolt_domain"

    udev_attrs = ['security']
    udev_props = ['DEVTYPE']

    SECURITY_NONE = 'none'
    SECURITY_USER = 'user'
    SECURITY_SECURE = 'secure'

    def __init__(self, security = SECURITY_SECURE, index = 0, host = None):
        assert host
        assert isinstance(host, TbHost)
        name = 'domain%d' % index
        super(TbDomain, self).__init__(name, children=[host])
        self.security = security

    @property
    def devices(self):
        return self.collect(lambda c: isinstance(c, TbDevice))

    @property
    def domain(self):
        return self

# Test Suite
class thunderbolt_test(unittest.TestCase):
    @classmethod
    def setUp(self):
        self.testbed = UMockdev.Testbed.new()
        print("\nPreparing test case\n")
        # Remove ACL database before each test case
        for root, dirs, files in os.walk(ACL, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))

    def tearDown(self):
        print(self)
        log.debug("Tear down test case")

    # mock tree stuff
    def default_mock_tree(self):
        # default mock tree
        tree = TbDomain(host=TbHost([
            TbDevice('0-1', device_name = DEVICE_NAME, vendor = VENDOR)]))
        return tree

    # Authorized security level 0 device tree
    def authorized_mock_tree(self, index = 0):
        # Tree with security level 0
        host = TbHost([TbDevice('%d-1' % index, device_name = DEVICE_NAME,
                      vendor = VENDOR, authorized = 1)], index = index)
        tree = TbDomain(security = TbDomain.SECURITY_NONE, index = index,
                        host = host)

        return tree

    # Parse tbtadm devices
    def get_device_line(self, route):
        u = subprocess.check_output(
                shlex.split("%s devices" % TBTADM)).decode("utf-8")
        lines = u.splitlines()
        for l in lines:
            if re.findall("^%s" % route, l):
                return l

    # Parse tbtadm topology
    def extract_property(self, u, prop):
        lines = u.splitlines()
        for l in lines:
            seclevel = re.findall(".*%s: (.*)" % prop, l)
            if seclevel:
                log.debug("%s: %s", prop, seclevel[0])
                return seclevel[0]

    def get_info(self):
        u = subprocess.check_output(
                shlex.split("%s topology" % TBTADM)).decode("utf-8")
        log.debug(u)
        return u

    def get_seclevel(self):
        return self.extract_property(self.get_info(), "Security level")

    def get_authorized(self):
        return self.extract_property(self.get_info(), "Authorized")

    def get_acl_status(self):
        return self.extract_property(self.get_info(), "In ACL")

    def get_uuid(self):
        return self.extract_property(self.get_info(), "UUID")

    # the actual tests
    def test_01_tbtadm_devices(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        output = self.get_device_line("0-1")
        log.debug(output)
        self.assertTrue(VENDOR in output)
        self.assertTrue(DEVICE_NAME in output)
        self.assertTrue("non-authorized" in output)
        self.assertTrue("not in ACL" in output)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Get security level through tbtadm topology
    def test_02_tbtadm_domain_seclevel(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        # Test default security secure (SL2)
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL2 (secure)")

        # Set security to None (SL0)
        tree.testbed.set_attribute("/sys/bus/thunderbolt/devices/domain0",
                                   "security", tree.SECURITY_NONE)

        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL0 (none)")

        # Set security to User (SL1)
        tree.testbed.set_attribute("/sys/bus/thunderbolt/devices/domain0",
                                   "security", tree.SECURITY_USER)

        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL1 (user)")

        # disconnect all devices
        tree.disconnect(self.testbed)

    def test_03_tbtadm_authorization_sl0(self):
        # connect all device
        tree = self.authorized_mock_tree()
        tree.connect_tree(self.testbed)

        # Check security is User (SL0)
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL0 (none)")

        # Check authorized
        authorized = self.get_authorized()
        self.assertEqual(authorized, "Yes")

        # Check ACL presence
        in_acl = self.get_acl_status()
        self.assertEqual(in_acl, "No")

        # Get uuid
        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        output = subprocess.check_output(shlex.split("%s approve-all" % TBTADM))
        self.assertTrue(b'Approval not relevant in SL0' in output)

        output = subprocess.check_output(shlex.split("%s approve --once 0-1" % TBTADM))
        self.assertTrue(b'Already authorized' in output)

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        self.assertTrue(b'Already authorized' in output)

        output = subprocess.check_output(shlex.split("%s add 0-1" % TBTADM))
        self.assertTrue(b'Adding to ACL is not relevant in SL0' in output)

        # ACL should not exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        output = subprocess.check_output(shlex.split("%s acl" % TBTADM))
        log.debug(output)
        self.assertTrue(b'ACL is empty' in output)

        output = subprocess.check_output(shlex.split("%s devices" % TBTADM))
        log.debug(output)

        output = self.get_device_line("0-1")
        self.assertTrue(VENDOR in output)
        self.assertTrue(DEVICE_NAME in output)
        self.assertFalse("non-authorized" in output)
        self.assertTrue("not in ACL" in output)

        # Test remove
        output = subprocess.check_output(shlex.split("%s remove 0-1" % TBTADM))
        self.assertTrue(b'ACL entry doesn\'t exist' in output)

        output = subprocess.check_output(shlex.split("%s remove-all" % TBTADM))
        self.assertTrue(b'ACL is empty' in output)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test authorization in SL1 (approve --once)
    def test_04_tbtadm_authorization_sl1(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        # Set security to User (SL1)
        tree.testbed.set_attribute(tree.syspath, "security", tree.SECURITY_USER)
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL1 (user)")

        authorized = self.get_authorized()
        self.assertEqual(authorized, "No")

        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        output = subprocess.check_output(shlex.split("%s approve --once 0-1" % TBTADM))
        self.assertTrue(b'Authorized' in output)

        authorized = self.get_authorized()
        self.assertEqual(authorized, "Yes")

        # ACL should not exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        # Test that second authorization returns "Already authorized"
        output = subprocess.check_output(shlex.split("%s approve --once 0-1" % TBTADM))
        self.assertTrue(b'Already authorized' in output)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test authorization and ACL management in SL1 mode
    def test_05_tbtadm_approve_sl1(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        # Set security to User (SL1)
        tree.testbed.set_attribute(tree.syspath, "security", tree.SECURITY_USER)
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL1 (user)")

        authorized = self.get_authorized()
        self.assertEqual(authorized, "No")

        in_acl = self.get_acl_status()
        self.assertEqual(in_acl, "No")

        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        # ACL should not yet exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        self.assertTrue(b'Authorized' in output)
        self.assertTrue(b'Added to ACL' in output)

        authorized = self.get_authorized()
        self.assertEqual(authorized, "Yes")

        in_acl = self.get_acl_status()
        self.assertEqual(in_acl, "Yes")

        # ACL entry should be created for given UUID
        self.assertTrue(os.path.isdir(ACL + "/" + uuid))

        # Verify content of ACL directory
        ls = os.listdir(ACL + "/" + uuid)
        ls.sort()
        self.assertTrue(ls == ['device_name', 'vendor_name'])

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test authorization in SL2 (approve --once)
    def test_06_tbtadm_authorization_sl2(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        # Check security level
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL2 (secure)")

        authorized = self.get_authorized()
        self.assertEqual(authorized, "No")

        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        output = subprocess.check_output(shlex.split("%s approve --once 0-1" % TBTADM))
        self.assertTrue(b'Authorized' in output)

        authorized = self.get_authorized()
        self.assertEqual(authorized, "Yes")

        # ACL should not exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        # Test that second authorization returns "Already authorized"
        output = subprocess.check_output(shlex.split("%s approve --once 0-1" % TBTADM))
        log.debug(output)
        self.assertTrue(b'Already authorized' in output)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test authorization and ACL management in SL2 mode
    def test_07_tbtadm_approve_sl2(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        # Check security level
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL2 (secure)")

        authorized = self.get_authorized()
        self.assertEqual(authorized, "No")

        in_acl = self.get_acl_status()
        self.assertEqual(in_acl, "No")

        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        # ACL should not yet exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        self.assertTrue(b'Authorized' in output)
        self.assertTrue(b'Added to ACL' in output)
        self.assertTrue(b'Key saved in ACL' in output)

        authorized = self.get_authorized()
        self.assertEqual(authorized, "Yes")

        in_acl = self.get_acl_status()
        self.assertEqual(in_acl, "Yes")

        # Check also "acl" command
        output = subprocess.check_output(shlex.split("%s acl" % TBTADM))
        self.assertTrue(str.encode(uuid) in output)

        # ACL entry should be created for given UUID
        self.assertTrue(os.path.isdir(ACL + "/" + uuid))

        # Verify content of ACL directory
        ls = os.listdir(ACL + "/" + uuid)
        ls.sort()
        self.assertTrue(ls == ['device_name', 'key', 'vendor_name'])

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test multi - controller device tree
    def test_08_multi_controller_topology(self):
        # connect all device
        device1 = TbDevice("0-2")
        device2 = TbDevice("0-1", children = [device1])
        tree1 = TbDomain(host = TbHost([device2]))
        tree1.connect_tree(self.testbed)

        device3 = TbDevice("1-1")
        tree2 = TbDomain(host = TbHost([device3], index = 1), index = 1)
        tree2.connect_tree(self.testbed)

        subprocess.run(shlex.split("%s topology" % TBTADM))

        # disconnect all devices
        tree1.disconnect(self.testbed)
        tree2.disconnect(self.testbed)

    # Generate boot_acl comma separated string
    def generate_boot_acl(self, *args):
        ls = list(args)
        for i in range(len(ls), 16):
            ls.append('')

        return ','.join(ls)

    # Add UUID to comma separated string
    def add_boot_acl(self, boot_acl, uuid):
        ls = boot_acl.split(',')

        # Remove empty uuids
        ls = ' '.join(ls).split()
        if len(ls) < 16:
            ls.append(uuid)
        else:
            ls.pop(0)
            ls.append(uuid)

        for i in range(len(ls), 16):
            ls.append('')

        return ','.join(ls)

    # Delete UUID from comma separated string
    def del_boot_acl(self, boot_acl, uuid):
        ls = boot_acl.split(',')

        # Remove empty uuids
        ls = ' '.join(ls).split()

        # Remove element by value
        if uuid in ls:
            ls.remove(uuid)

        for i in range(len(ls), 16):
            ls.append('')

        return ','.join(ls)

    # Test preboot_acl
    def test_09_preboot_acl(self):
        # connect all device
        tree = self.default_mock_tree()
        tree.connect_tree(self.testbed)

        SYSFS_BOOT_ACL = SYSFS + "/devices/domain0/boot_acl"

        # Generate empty boot_acl
        boot_acl = self.generate_boot_acl()
        self.assertEqual(len(boot_acl.split(',')), 16)

        # Add new attribute to domain tree
        tree.testbed.set_attribute(tree.syspath, "boot_acl", boot_acl)

        # Test that sysfs entry created
        self.assertTrue(os.path.isfile(SYSFS_BOOT_ACL))

        # Verify content of boot_acl
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl)

        # Check security level
        seclevel = self.get_seclevel()
        self.assertEqual(seclevel, "SL2 (secure)")

        authorized = self.get_authorized()
        self.assertEqual(authorized, "No")

        uuid = self.get_uuid()
        self.assertNotEqual(uuid, None)

        # ACL should not yet exist
        self.assertFalse(os.path.isdir(ACL + "/" + uuid))

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        log.debug(output)

        # Verify adding UUID to empty list
        boot_acl_generated = self.generate_boot_acl(uuid)
        self.assertEqual(len(boot_acl_generated.split(',')), 16)
        log.debug(boot_acl_generated)

        boot_acl_added = self.add_boot_acl(boot_acl, uuid)
        self.assertEqual(len(boot_acl_added.split(',')), 16)
        log.debug(boot_acl_added)

        # Two ways of generation are the same ;)
        self.assertEqual(boot_acl_generated, boot_acl_added)

        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_generated)

        # Test also removing from Boot ACL
        output = subprocess.check_output(shlex.split("%s remove 0-1" % TBTADM))
        log.debug(output)

        boot_acl_deleted = self.del_boot_acl(boot_acl_added, uuid)
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_deleted)
        log.debug("open boot_acl %s" % open(SYSFS_BOOT_ACL).read())

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test adding and removing UUID to boot_acl with existing 16 entries
    def test_10_preboot_acl_full_list(self):
        # connect all device
        UUID = '00000000-0000-0000-0000-000000000001'
        device1 = TbDevice("0-1", uid = UUID)
        device2 = TbDevice("0-2", children = [device1])
        tree = TbDomain(host = TbHost([device2]))
        tree.connect_tree(self.testbed)

        SYSFS_BOOT_ACL = SYSFS + "/devices/domain0/boot_acl"

        # Generate uuid comma separated list
        boot_acl = self.generate_boot_acl(
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()))

        self.assertEqual(len(boot_acl.split(',')), 16)
        log.debug(boot_acl)

        # Add new attribute to domain tree
        tree.testbed.set_attribute(tree.syspath, "boot_acl", boot_acl)

        # Test that sysfs entry created
        self.assertTrue(os.path.isfile(SYSFS_BOOT_ACL))

        # Validate sysfs
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl)

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        self.assertEqual(len(open(SYSFS_BOOT_ACL).read().split(',')), 16)
        log.debug(output)

        boot_acl_added = self.add_boot_acl(boot_acl, UUID)
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_added)

        # Test removing UUID
        output = subprocess.check_output(shlex.split("%s remove 0-1" % TBTADM))
        log.debug(output)

        log.debug("read boot_acl %s" % open(SYSFS_BOOT_ACL).read())

        # Verify boot_acl after removing element
        boot_acl_deleted = self.del_boot_acl(boot_acl_added, UUID)
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_deleted)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test removing from boot_acl with non present entry to be removed
    def test_11_preboot_acl_remove_not_present(self):
        # connect all device
        UUID = '00000000-0000-0000-0000-000000000001'
        device1 = TbDevice("0-1", uid = UUID)
        device2 = TbDevice("0-2", children = [device1])
        tree = TbDomain(host = TbHost([device2]))
        tree.connect_tree(self.testbed)

        SYSFS_BOOT_ACL = SYSFS + "/devices/domain0/boot_acl"

        # Generate uuid comma separated list
        boot_acl = self.generate_boot_acl(
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()))

        self.assertEqual(len(boot_acl.split(',')), 16)
        log.debug(boot_acl)

        # Add new attribute to domain tree
        tree.testbed.set_attribute(tree.syspath, "boot_acl", boot_acl)

        # Test that sysfs entry created
        self.assertTrue(os.path.isfile(SYSFS_BOOT_ACL))

        # Validate sysfs
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl)

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        self.assertEqual(len(open(SYSFS_BOOT_ACL).read().split(',')), 16)
        log.debug(output)

        boot_acl_added = self.add_boot_acl(boot_acl, UUID)
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_added)

        # Simulate boot_acl without added device
        tree.testbed.set_attribute(tree.syspath, "boot_acl", boot_acl)

        # Test removing UUID
        output = subprocess.check_output(shlex.split("%s remove 0-1" % TBTADM))
        log.debug(output)

        log.debug("read boot_acl %s" % open(SYSFS_BOOT_ACL).read())

        # Verify boot_acl after removing element
        boot_acl_deleted = self.del_boot_acl(boot_acl, UUID)
        self.assertEqual(open(SYSFS_BOOT_ACL).read(), boot_acl_deleted)

        # disconnect all devices
        tree.disconnect(self.testbed)

    # Test multi controllers with preboot_acl
    def test_12_preboot_acl_multi_controller(self):
        # connect all device
        # First controller
        UUID = '00000000-0000-0000-0000-000000000001'
        device1 = TbDevice("0-2")
        device2 = TbDevice("0-1", uid = UUID, children = [device1])
        tree1 = TbDomain(host = TbHost([device2]))

        # Second controller
        device3 = TbDevice("1-1")
        tree2 = TbDomain(host = TbHost([device3], index = 1), index = 1)

        tree1.connect_tree(self.testbed)
        tree2.connect_tree(self.testbed)

        SYSFS_DOMAIN0_BOOT_ACL = SYSFS + "/devices/domain0/boot_acl"
        SYSFS_DOMAIN1_BOOT_ACL = SYSFS + "/devices/domain1/boot_acl"

        # Generate uuid comma separated list
        boot_acl = self.generate_boot_acl(
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()), str(uuid.uuid4()), str(uuid.uuid4()),
                str(uuid.uuid4()))

        self.assertEqual(len(boot_acl.split(',')), 16)
        log.debug(boot_acl)

        # Add new attribute to domain trees
        tree1.testbed.set_attribute(tree1.syspath, "boot_acl", boot_acl)
        tree2.testbed.set_attribute(tree2.syspath, "boot_acl", boot_acl)

        # Test that sysfs entry created for each domain
        self.assertTrue(os.path.isfile(SYSFS_DOMAIN0_BOOT_ACL))
        self.assertTrue(os.path.isfile(SYSFS_DOMAIN1_BOOT_ACL))

        # Validate sysfs
        self.assertEqual(open(SYSFS_DOMAIN0_BOOT_ACL).read(), boot_acl)
        self.assertEqual(open(SYSFS_DOMAIN1_BOOT_ACL).read(), boot_acl)

        subprocess.run(shlex.split("%s topology" % TBTADM))

        output = subprocess.check_output(shlex.split("%s approve 0-1" % TBTADM))
        log.debug(output)
        self.assertEqual(len(open(SYSFS_DOMAIN0_BOOT_ACL).read().split(',')), 16)

        boot_acl_added = self.add_boot_acl(boot_acl, UUID)
        self.assertEqual(open(SYSFS_DOMAIN0_BOOT_ACL).read(), boot_acl_added)
        self.assertEqual(open(SYSFS_DOMAIN1_BOOT_ACL).read(), boot_acl_added)

        log.debug(open(SYSFS_DOMAIN0_BOOT_ACL).read())
        log.debug(open(SYSFS_DOMAIN1_BOOT_ACL).read())

        tree1.disconnect(self.testbed)
        tree2.disconnect(self.testbed)

if __name__ == '__main__':
    # run ourselves under umockdev
    if 'umockdev' not in os.environ.get('LD_PRELOAD', ''):
        os.execvp('umockdev-wrapper', ['umockdev-wrapper'] + sys.argv)

    loglevel = logging.DEBUG
    logging.basicConfig(level=loglevel)

    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))
