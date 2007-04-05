# schroot Makefile template
#
#
# Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>
#
# schroot is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# schroot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307  USA
#
#####################################################################

include $(top_srcdir)/scripts/global.mk

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) -I$(top_srcdir)/bin

DEFS = -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

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
	dchroot-chroot-config.h		\
	dchroot-chroot-config.cc	\
	dchroot-session.h		\
	dchroot-session.cc		\
	dchroot-options.h		\
	dchroot-options.cc		\
	dchroot-main.h			\
	dchroot-main.cc			\
	dchroot.cc
dchroot_LDADD = libdchroot.la

if BUILD_DCHROOT
dchroot_mans = dchroot.1
endif

man_MANS = $(dchroot_mans)

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/dchroot" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot"; \
	fi
