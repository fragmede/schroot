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

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\" -DCSBUILD_CONF=\"$(CSBUILD_CONF)\"

if BUILD_CSBUILD
csbuild = csbuild
endif

bin_PROGRAMS = $(csbuild)

csbuild_SOURCES =			\
	csbuild-debian-changes.h	\
	csbuild-debian-changes.cc	\
	csbuild-options.h		\
	csbuild-options.cc		\
	csbuild-main.h			\
	csbuild-main.cc			\
	csbuild.cc
csbuild_LDADD = $(top_builddir)/bin/schroot/libschroot.la

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/csbuild" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/csbuild"; \
	fi
