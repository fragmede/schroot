# schroot Makefile template
#
#
# Copyright © 2004-2009  Roger Leigh <rleigh@debian.org>
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

include $(top_srcdir)/scripts/global.mk

localedir = $(datadir)/locale

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) -I$(top_srcdir)/bin

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\"

noinst_LTLIBRARIES = libschroot.la libschroot-all.la

bin_PROGRAMS = schroot

libschroot_la_SOURCES =			\
	schroot-options-base.h		\
	schroot-options-base.cc		\
	schroot-main-base.h		\
	schroot-main-base.cc		\
	schroot-main.h			\
	schroot-main.cc
libschroot_la_LIBADD = $(top_builddir)/bin/schroot-base/libschroot-base.la

libschroot_all_la_SOURCES =		\
	schroot-options.h		\
	schroot-options.cc
libschroot_all_la_LIBADD = libschroot.la

schroot_SOURCES =			\
	schroot.cc
schroot_LDADD = libschroot-all.la

install-exec-hook:
# Install setuid root.
	chmod 4755 "$(DESTDIR)$(bindir)/schroot"

install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(schroot_sysconf_chrootdir)
	$(MKDIR_P) $(DESTDIR)$(schroot_sessiondir)
	$(MKDIR_P) $(DESTDIR)$(schroot_file_unpackdir)
	$(MKDIR_P) $(DESTDIR)$(schroot_overlaydir)
	$(MKDIR_P) $(DESTDIR)$(schroot_underlaydir)

EXTRA_DIST = CMakeLists.txt
