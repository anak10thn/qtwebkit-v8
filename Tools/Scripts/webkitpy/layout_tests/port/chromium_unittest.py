# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import StringIO
import sys
import time
import unittest

from webkitpy.common.system import logtesting
from webkitpy.common.system.executive_mock import MockExecutive, MockExecutive2
from webkitpy.common.system.filesystem_mock import MockFileSystem
from webkitpy.common.system.systemhost_mock import MockSystemHost
from webkitpy.layout_tests.port.config_mock import MockConfig
from webkitpy.thirdparty.mock import Mock
from webkitpy.tool.mocktool import MockOptions

import chromium
import chromium_android
import chromium_linux
import chromium_mac
import chromium_win

from webkitpy.layout_tests.models.test_configuration import TestConfiguration
from webkitpy.layout_tests.port import port_testcase
from webkitpy.layout_tests.port.driver import DriverInput


class ChromiumDriverTest(unittest.TestCase):
    def setUp(self):
        host = MockSystemHost()
        options = MockOptions(configuration='Release', additional_drt_flag=['--test-shell'])
        config = MockConfig(filesystem=host.filesystem, default_configuration='Release')
        self.port = chromium_mac.ChromiumMacPort(host, 'chromium-mac-snowleopard', options=options, config=config)
        self.driver = chromium.ChromiumDriver(self.port, worker_number=0, pixel_tests=True)

    def test_test_shell_command(self):
        expected_command = "test.html 2 checksum\n"
        self.assertEqual(self.driver._test_shell_command("test.html", 2, "checksum"), expected_command)

    def _assert_write_command_and_read_line(self, input=None, expected_line=None, expected_stdin=None, expected_crash=False):
        if not expected_stdin:
            if input:
                expected_stdin = input
            else:
                # We reset stdin, so we should expect stdin.getValue = ""
                expected_stdin = ""
        self.driver._proc.stdin = StringIO.StringIO()
        line, did_crash = self.driver._write_command_and_read_line(input)
        self.assertEqual(self.driver._proc.stdin.getvalue(), expected_stdin)
        self.assertEqual(line, expected_line)
        self.assertEqual(did_crash, expected_crash)

    def test_write_command_and_read_line(self):
        self.driver._proc = Mock()  # FIXME: This should use a tighter mock.
        # Set up to read 3 lines before we get an IOError
        self.driver._proc.stdout = StringIO.StringIO("first\nsecond\nthird\n")

        unicode_input = u"I \u2661 Unicode"
        utf8_input = unicode_input.encode("utf-8")
        # Test unicode input conversion to utf-8
        self._assert_write_command_and_read_line(input=unicode_input, expected_stdin=utf8_input, expected_line="first\n")
        # Test str() input.
        self._assert_write_command_and_read_line(input="foo", expected_line="second\n")
        # Test input=None
        self._assert_write_command_and_read_line(expected_line="third\n")
        # Test reading from a closed/empty stream.
        # reading from a StringIO does not raise IOError like a real file would, so raise IOError manually.
        def mock_readline():
            raise IOError
        self.driver._proc.stdout.readline = mock_readline
        self._assert_write_command_and_read_line(expected_crash=True)

    def test_crash_log(self):
        self.driver._proc = Mock()

        # Simulate a crash by having stdout close unexpectedly.
        def mock_readline():
            raise IOError
        self.driver._proc.stdout.readline = mock_readline
        self.driver._proc.pid = 1234

        self.driver.test_to_uri = lambda test: 'mocktesturi'
        self.driver._port.driver_name = lambda: 'mockdriver'
        self.driver._port._get_crash_log = lambda name, pid, out, err, newer_than: 'mockcrashlog'
        driver_output = self.driver.run_test(DriverInput(test_name='some/test.html', timeout=1, image_hash=None, should_run_pixel_test=False))
        self.assertTrue(driver_output.crash)
        self.assertEqual(driver_output.crashed_process_name, 'mockdriver')
        self.assertEqual(driver_output.crashed_pid, 1234)
        self.assertEqual(driver_output.crash_log, 'mockcrashlog')

    def test_stop(self):
        self.pid = None
        self.wait_called = False
        self.driver._proc = Mock()  # FIXME: This should use a tighter mock.
        self.driver._proc.pid = 1
        self.driver._proc.stdin = StringIO.StringIO()
        self.driver._proc.stdout = StringIO.StringIO()
        self.driver._proc.stderr = StringIO.StringIO()
        self.driver._proc.poll = lambda: None

        def fake_wait():
            self.assertTrue(self.pid is not None)
            self.wait_called = True

        self.driver._proc.wait = fake_wait

        class FakeExecutive(object):
            def kill_process(other, pid):
                self.pid = pid
                self.driver._proc.poll = lambda: 2

        self.driver._port._executive = FakeExecutive()
        self.driver.KILL_TIMEOUT_DEFAULT = 0.01
        self.driver.stop()
        self.assertTrue(self.wait_called)
        self.assertEquals(self.pid, 1)

    def test_two_drivers(self):

        class MockDriver(chromium.ChromiumDriver):
            def __init__(self, port):
                chromium.ChromiumDriver.__init__(self, port, worker_number=0, pixel_tests=False)

            def cmd_line(self, pixel_test, per_test_args):
                return 'python'

        # get_option is used to get the timeout (ms) for a process before we kill it.
        driver1 = MockDriver(self.port)
        driver1._start(False, [])
        driver2 = MockDriver(self.port)
        driver2._start(False, [])
        # It's possible for driver1 to timeout when stopping if it's sharing stdin with driver2.
        start_time = time.time()
        driver1.stop()
        driver2.stop()
        self.assertTrue(time.time() - start_time < 20)


class ChromiumPortTest(port_testcase.PortTestCase):
    port_name = 'chromium-mac'
    port_maker = chromium.ChromiumPort

    def test_all_test_configurations(self):
        """Validate the complete set of configurations this port knows about."""
        port = self.make_port()
        self.assertEquals(set(port.all_test_configurations()), set([
            TestConfiguration('icecreamsandwich', 'x86', 'debug'),
            TestConfiguration('icecreamsandwich', 'x86', 'release'),
            TestConfiguration('leopard', 'x86', 'debug'),
            TestConfiguration('leopard', 'x86', 'release'),
            TestConfiguration('snowleopard', 'x86', 'debug'),
            TestConfiguration('snowleopard', 'x86', 'release'),
            TestConfiguration('lion', 'x86', 'debug'),
            TestConfiguration('lion', 'x86', 'release'),
            TestConfiguration('xp', 'x86', 'debug'),
            TestConfiguration('xp', 'x86', 'release'),
            TestConfiguration('vista', 'x86', 'debug'),
            TestConfiguration('vista', 'x86', 'release'),
            TestConfiguration('win7', 'x86', 'debug'),
            TestConfiguration('win7', 'x86', 'release'),
            TestConfiguration('lucid', 'x86', 'debug'),
            TestConfiguration('lucid', 'x86', 'release'),
            TestConfiguration('lucid', 'x86_64', 'debug'),
            TestConfiguration('lucid', 'x86_64', 'release'),
        ]))

    def test_driver_cmd_line(self):
        # Override this test since ChromiumPort doesn't implement driver_cmd_line().
        pass

    def test_check_build(self):
        # Override this test since ChromiumPort doesn't implement _path_to_driver().
        pass

    def test_check_wdiff(self):
        # Override this test since ChromiumPort doesn't implement _path_to_driver().
        pass

    class TestMacPort(chromium_mac.ChromiumMacPort):
        def __init__(self, options=None):
            options = options or MockOptions()
            chromium_mac.ChromiumMacPort.__init__(self, MockSystemHost(os_name='mac', os_version='leopard'), 'chromium-mac-leopard', options=options)

        def default_configuration(self):
            self.default_configuration_called = True
            return 'default'

    class TestAndroidPort(chromium_android.ChromiumAndroidPort):
        def __init__(self, options=None):
            options = options or MockOptions()
            chromium_win.ChromiumAndroidPort.__init__(self, MockSystemHost(os_name='android', os_version='icecreamsandwich'), 'chromium-android', options=options)

        def default_configuration(self):
            self.default_configuration_called = True
            return 'default'

    class TestLinuxPort(chromium_linux.ChromiumLinuxPort):
        def __init__(self, options=None):
            options = options or MockOptions()
            chromium_linux.ChromiumLinuxPort.__init__(self, MockSystemHost(os_name='linux', os_version='lucid'), 'chromium-linux-x86', options=options)

        def default_configuration(self):
            self.default_configuration_called = True
            return 'default'

    class TestWinPort(chromium_win.ChromiumWinPort):
        def __init__(self, options=None):
            options = options or MockOptions()
            chromium_win.ChromiumWinPort.__init__(self, MockSystemHost(os_name='win', os_version='xp'), 'chromium-win-xp', options=options)

        def default_configuration(self):
            self.default_configuration_called = True
            return 'default'

    def test_path_to_image_diff(self):
        # FIXME: These don't need to use endswith now that the port uses a MockFileSystem.
        self.assertTrue(ChromiumPortTest.TestLinuxPort()._path_to_image_diff().endswith('/out/default/ImageDiff'))
        self.assertTrue(ChromiumPortTest.TestMacPort()._path_to_image_diff().endswith('/xcodebuild/default/ImageDiff'))
        self.assertTrue(ChromiumPortTest.TestWinPort()._path_to_image_diff().endswith('/default/ImageDiff.exe'))

    def test_default_configuration(self):
        mock_options = MockOptions()
        port = ChromiumPortTest.TestLinuxPort(options=mock_options)
        self.assertEquals(mock_options.configuration, 'default')
        self.assertTrue(port.default_configuration_called)

        mock_options = MockOptions(configuration=None)
        port = ChromiumPortTest.TestLinuxPort(mock_options)
        self.assertEquals(mock_options.configuration, 'default')
        self.assertTrue(port.default_configuration_called)

    def test_diff_image(self):
        class TestPort(ChromiumPortTest.TestLinuxPort):
            def _path_to_image_diff(self):
                return "/path/to/image_diff"

        port = ChromiumPortTest.TestLinuxPort()
        mock_image_diff = "MOCK Image Diff"

        def mock_run_command(args):
            port._filesystem.write_binary_file(args[4], mock_image_diff)
            return 1

        # Images are different.
        port._executive = MockExecutive2(run_command_fn=mock_run_command)
        self.assertEquals(mock_image_diff, port.diff_image("EXPECTED", "ACTUAL")[0])

        # Images are the same.
        port._executive = MockExecutive2(exit_code=0)
        self.assertEquals(None, port.diff_image("EXPECTED", "ACTUAL")[0])

        # There was some error running image_diff.
        port._executive = MockExecutive2(exit_code=2)
        exception_raised = False
        try:
            port.diff_image("EXPECTED", "ACTUAL")
        except ValueError, e:
            exception_raised = True
        self.assertFalse(exception_raised)

    def test_overrides_and_builder_names(self):
        port = self.make_port()

        filesystem = MockFileSystem()
        port._filesystem = filesystem
        port.path_from_chromium_base = lambda *comps: '/' + '/'.join(comps)

        chromium_overrides_path = port.path_from_chromium_base(
            'webkit', 'tools', 'layout_tests', 'test_expectations.txt')
        CHROMIUM_OVERRIDES = 'contents of %s\n' % chromium_overrides_path
        filesystem.write_text_file(chromium_overrides_path, CHROMIUM_OVERRIDES)
        skia_overrides_path = port.path_from_chromium_base(
            'skia', 'skia_test_expectations.txt')
        SKIA_OVERRIDES = 'contents of %s\n' % skia_overrides_path
        filesystem.write_text_file(skia_overrides_path, SKIA_OVERRIDES)

        additional_expectations_path = port.path_from_chromium_base(
            'additional_expectations.txt')
        ADDITIONAL_EXPECTATIONS = 'contents of %s\n' % additional_expectations_path
        filesystem.write_text_file(additional_expectations_path, ADDITIONAL_EXPECTATIONS)

        port._options.builder_name = 'DUMMY_BUILDER_NAME'
        port._options.additional_expectations = []
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES + CHROMIUM_OVERRIDES)
        port._options.additional_expectations = [additional_expectations_path]
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES + CHROMIUM_OVERRIDES + ADDITIONAL_EXPECTATIONS)

        port._options.builder_name = 'builder (deps)'
        port._options.additional_expectations = []
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES + CHROMIUM_OVERRIDES)
        port._options.additional_expectations = [additional_expectations_path]
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES + CHROMIUM_OVERRIDES + ADDITIONAL_EXPECTATIONS)

        # A builder which does NOT observe the Chromium test_expectations,
        # but still observes the Skia test_expectations...
        port._options.builder_name = 'builder'
        port._options.additional_expectations = []
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES)
        port._options.additional_expectations = [additional_expectations_path]
        self.assertEquals(port.test_expectations_overrides(),
                          SKIA_OVERRIDES + ADDITIONAL_EXPECTATIONS)

    def test_driver_name_option(self):
        self.assertTrue(ChromiumPortTest.TestLinuxPort()._path_to_driver().endswith('/out/default/DumpRenderTree'))
        self.assertTrue(ChromiumPortTest.TestMacPort()._path_to_driver().endswith('/xcodebuild/default/DumpRenderTree.app/Contents/MacOS/DumpRenderTree'))
        self.assertTrue(ChromiumPortTest.TestWinPort()._path_to_driver().endswith('/default/DumpRenderTree.exe'))

        options = MockOptions(driver_name='OtherDriver')
        self.assertTrue(ChromiumPortTest.TestLinuxPort(options)._path_to_driver().endswith('/out/default/OtherDriver'))
        self.assertTrue(ChromiumPortTest.TestMacPort(options)._path_to_driver().endswith('/xcodebuild/default/OtherDriver.app/Contents/MacOS/OtherDriver'))
        self.assertTrue(ChromiumPortTest.TestWinPort(options)._path_to_driver().endswith('/default/OtherDriver.exe'))


class ChromiumPortLoggingTest(logtesting.LoggingTestCase):
    def test_check_sys_deps(self):
        port = ChromiumPortTest.TestLinuxPort()

        # Success
        port._executive = MockExecutive2(exit_code=0)
        self.assertTrue(port.check_sys_deps(needs_http=False))

        # Failure
        port._executive = MockExecutive2(exit_code=1,
            output='testing output failure')
        self.assertFalse(port.check_sys_deps(needs_http=False))
        self.assertLog([
            'ERROR: System dependencies check failed.\n',
            'ERROR: To override, invoke with --nocheck-sys-deps\n',
            'ERROR: \n',
            'ERROR: testing output failure\n'])


if __name__ == '__main__':
    unittest.main()
