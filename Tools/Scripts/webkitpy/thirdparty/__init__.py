# Copyright (C) 2010 Chris Jerdonek (cjerdonek@webkit.org)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This module is required for Python to treat this directory as a package.

"""Autoinstalls third-party code required by WebKit."""


import codecs
import os
import sys

from webkitpy.common.system.autoinstall import AutoInstaller
from webkitpy.common.system.filesystem import FileSystem

_THIRDPARTY_DIR = os.path.dirname(__file__)
_AUTOINSTALLED_DIR = os.path.join(_THIRDPARTY_DIR, "autoinstalled")

# Putting the autoinstall code into webkitpy/thirdparty/__init__.py
# ensures that no autoinstalling occurs until a caller imports from
# webkitpy.thirdparty.  This is useful if the caller wants to configure
# logging prior to executing autoinstall code.

# FIXME: If any of these servers is offline, webkit-patch breaks (and maybe
# other scripts do, too). See <http://webkit.org/b/42080>.

# We put auto-installed third-party modules in this directory--
#
#     webkitpy/thirdparty/autoinstalled
fs = FileSystem()
fs.maybe_make_directory(_AUTOINSTALLED_DIR)

init_path = fs.join(_AUTOINSTALLED_DIR, "__init__.py")
if not fs.exists(init_path):
    fs.write_text_file(init_path, "")

readme_path = fs.join(_AUTOINSTALLED_DIR, "README")
if not fs.exists(readme_path):
    fs.write_text_file(readme_path,
        "This directory is auto-generated by WebKit and is "
        "safe to delete.\nIt contains needed third-party Python "
        "packages automatically downloaded from the web.")


class AutoinstallImportHook(object):
    def __init__(self, filesystem=None):
        self._fs = filesystem or FileSystem()

    def find_module(self, fullname, path):
        # This method will run before each import. See http://www.python.org/dev/peps/pep-0302/
        if '.autoinstalled' not in fullname:
            return

        if '.mechanize' in fullname:
            self._install_mechanize()
        elif '.pep8' in fullname:
            self._install_pep8()
        elif '.coverage' in fullname:
            self._install_coverage()
        elif '.eliza' in fullname:
            self._install_eliza()
        elif '.irc' in fullname:
            self._install_irc()
        elif '.buildbot' in fullname:
            self._install_buildbot()

    def _install_mechanize(self):
        self._install("http://pypi.python.org/packages/source/m/mechanize/mechanize-0.2.5.tar.gz",
                      "mechanize-0.2.5/mechanize")

    def _install_pep8(self):
        self._install("http://pypi.python.org/packages/source/p/pep8/pep8-0.5.0.tar.gz#md5=512a818af9979290cd619cce8e9c2e2b",
                      "pep8-0.5.0/pep8.py")

    # autoinstalled.buildbot is used by BuildSlaveSupport/build.webkit.org-config/mastercfg_unittest.py
    # and should ideally match the version of BuildBot used at build.webkit.org.
    def _install_buildbot(self):
        # The buildbot package uses jinja2, for example, in buildbot/status/web/base.py.
        # buildbot imports jinja2 directly (as though it were installed on the system),
        # so the search path needs to include jinja2.  We put jinja2 in
        # its own directory so that we can include it in the search path
        # without including other modules as a side effect.
        jinja_dir = self._fs.join(_AUTOINSTALLED_DIR, "jinja2")
        installer = AutoInstaller(append_to_search_path=True, target_dir=jinja_dir)
        installer.install(url="http://pypi.python.org/packages/source/J/Jinja2/Jinja2-2.6.tar.gz#md5=1c49a8825c993bfdcf55bb36897d28a2",
                          url_subpath="Jinja2-2.6/jinja2")

        self._install("http://pypi.python.org/packages/source/b/buildbot/buildbot-0.8.4p2.tar.gz#md5=7597d945724c80c0ab476e833a1026cb", "buildbot-0.8.4p2/buildbot")

    def _install_coverage(self):
        installer = AutoInstaller(target_dir=_AUTOINSTALLED_DIR)
        installer.install(url="http://pypi.python.org/packages/source/c/coverage/coverage-3.5.1.tar.gz#md5=410d4c8155a4dab222f2bc51212d4a24", url_subpath="coverage-3.5.1/coverage")

    def _install_eliza(self):
        installer = AutoInstaller(target_dir=_AUTOINSTALLED_DIR)
        installer.install(url="http://www.adambarth.com/webkit/eliza",
                          target_name="eliza.py")

    def _install_irc(self):
        # Since irclib and ircbot are two top-level packages, we need to import
        # them separately.  We group them into an irc package for better
        # organization purposes.
        irc_dir = self._fs.join(_AUTOINSTALLED_DIR, "irc")
        installer = AutoInstaller(target_dir=irc_dir)
        installer.install(url="http://downloads.sourceforge.net/project/python-irclib/python-irclib/0.4.8/python-irclib-0.4.8.zip",
                          url_subpath="irclib.py")
        installer.install(url="http://downloads.sourceforge.net/project/python-irclib/python-irclib/0.4.8/python-irclib-0.4.8.zip",
                          url_subpath="ircbot.py")

    def _install(self, url, url_subpath):
        installer = AutoInstaller(target_dir=_AUTOINSTALLED_DIR)
        installer.install(url=url, url_subpath=url_subpath)


sys.meta_path.append(AutoinstallImportHook())
