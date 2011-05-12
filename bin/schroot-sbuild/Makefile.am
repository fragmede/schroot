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

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\"

schroot_sbuild = schroot-sbuild

bin_PROGRAMS = $(schroot_sbuild)

schroot_sbuild_SOURCES =                        \
	schroot-sbuild-session.h		\
	schroot-sbuild-session.cc		\
	schroot-sbuild-main.h			\
	schroot-sbuild-main.cc			\
	schroot-sbuild.cc
schroot_sbuild_LDADD = $(top_builddir)/bin/schroot/libschroot-all.la

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/schroot-sbuild" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/schroot-sbuild"; \
	fi
