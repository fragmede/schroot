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

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) -I$(top_srcdir)/bin

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\" -DDCHROOT_CONF=\"$(DCHROOT_CONF)\"

if BUILD_LIBDCHROOT
noinst_LTLIBRARIES = libdchroot.la
endif

if BUILD_DCHROOT
dchroot = dchroot
endif

bin_PROGRAMS = $(dchroot)

libdchroot_la_SOURCES =			\
	dchroot-main-base.h		\
	dchroot-main-base.cc		\
	dchroot-session-base.h		\
	dchroot-session-base.cc
libdchroot_la_LIBADD = $(top_builddir)/bin/schroot/libschroot.la

dchroot_SOURCES =			\
	dchroot-session.h		\
	dchroot-session.cc		\
	dchroot-options.h		\
	dchroot-options.cc		\
	dchroot-main.h			\
	dchroot-main.cc			\
	dchroot.cc
dchroot_LDADD = libdchroot.la

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/dchroot" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot"; \
	fi

EXTRA_DIST = CMakeLists.txt
