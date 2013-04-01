# Copyright © 2004-2013  Roger Leigh <rleigh@debian.org>
#
# schroot is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# schroot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#
#####################################################################

ACLOCAL_AMFLAGS=-I m4

MAKEFLAGS = -rR --no-print-directory

include $(top_srcdir)/scripts/global.mk
include $(top_srcdir)/scripts/git-dist.mk

localedir = $(datadir)/locale

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) $(CPPUNIT_CFLAGS) -DTESTDATADIR='"test/testdata"' -I$(top_srcdir)/bin

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\" -DDCHROOT_CONF=\"$(DCHROOT_CONF)\" -DCSBUILD_CONF=\"$(CSBUILD_CONF)\"

bin_PROGRAMS =
pkglibexec_PROGRAMS =
noinst_LTLIBRARIES =
noinst_PROGRAMS =
noinst_SCRIPTS =
schroot_sysconf_DATA =
man_MANS =
CLEANFILES =
EXTRA_DIST =

include etc/Makefile.am
include etc/pam/Makefile.am
include etc/setup.d/Makefile.am
include etc/bash_completion/Makefile.am
include contrib/setup.d/Makefile.am
include doc/Makefile.am
include doc/historical/Makefile.am
include man/Makefile.am
include sbuild/Makefile.am
include bin/schroot-listmounts/Makefile.am
include bin/dchroot-dsa/Makefile.am
include bin/csbuild/Makefile.am
include bin/dchroot/Makefile.am
include bin/schroot-sbuild/Makefile.am
include bin/schroot-base/Makefile.am
include bin/schroot-mount/Makefile.am
include bin/schroot/Makefile.am
include test/Makefile.am

SUBDIRS = po

dist-hook::
# Remove junk from the generated tarball.
	$(FIND) $(distdir) -name '.*' -print | $(XARGS) xargs rm -rf
	$(FIND) $(distdir) -name '#*' -print | $(XARGS) rm -rf
	$(FIND) $(distdir) -name '*~' -print | $(XARGS) rm -rf

# Generate ChangeLog
	if [ -d .git ]; then			\
	  git log > $(distdir)/ChangeLog;	\
	fi

EXTRA_DIST +=				\
	bootstrap			\
	debian				\
	scripts/schroot_release.m4.in	\
	m4				\
	HACKING				\
	VERSION
