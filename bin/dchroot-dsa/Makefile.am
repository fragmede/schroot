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

if BUILD_DCHROOT_DSA
dchroot_dsa = dchroot-dsa
endif

bin_PROGRAMS = $(dchroot_dsa)

dchroot_dsa_SOURCES =			\
	dchroot-dsa-session.h		\
	dchroot-dsa-session.cc		\
	dchroot-dsa-options.h		\
	dchroot-dsa-options.cc		\
	dchroot-dsa-main.h		\
	dchroot-dsa-main.cc		\
	dchroot-dsa.cc
dchroot_dsa_LDADD = $(top_builddir)/bin/dchroot/libdchroot.la

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/dchroot-dsa" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot-dsa"; \
	fi

EXTRA_DIST = CMakeLists.txt
