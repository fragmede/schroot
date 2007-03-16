# schroot Makefile template
#
#
# Copyright © 2004-2006  Roger Leigh <rleigh@debian.org>
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

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS)

DEFS = -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

if BUILD_CSBUILD
noinst_LTLIBRARIES = libcsbuild.la
csbuild = csbuild
endif

bin_PROGRAMS = $(csbuild)

libcsbuild_la_SOURCES =			\
	csbuild-main-base.h		\
	csbuild-main-base.cc		\
	csbuild-session-base.h		\
	csbuild-session-base.cc
libcsbuild_la_LIBADD = $(top_builddir)/schroot/libschroot.la

csbuild_SOURCES =			\
	csbuild-options.h		\
	csbuild-options.cc		\
	csbuild-main.h			\
	csbuild-main.cc			\
	csbuild.cc
csbuild_LDADD = libcsbuild.la

if BUILD_CSBUILD
csbuild_mans = csbuild.1
endif

man_MANS = $(csbuild_mans)

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/csbuild" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/csbuild"; \
	fi
