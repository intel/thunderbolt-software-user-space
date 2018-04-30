from test_integration_mock import *

l = (dir(thunderbolt_test))
tests = [ x for x in l if x.startswith('test')]

for test in tests:
    cmdline = "umockdev-wrapper python3 tests/test-integration-mock.py thunderbolt_test.%s" % test
    result = subprocess.run(shlex.split(cmdline))

    if result.returncode:
        exit(1)
