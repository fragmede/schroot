# schroot Makefile template
#
#
# Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>
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

pkglibexec_PROGRAMS = schroot-listmounts

schroot_listmounts_SOURCES =		\
	schroot-listmounts-main.h	\
	schroot-listmounts-main.cc	\
	schroot-listmounts-options.h	\
	schroot-listmounts-options.cc	\
	schroot-listmounts.cc
schroot_listmounts_LDADD = $(top_builddir)/bin/schroot-base/libschroot-base.la

EXTRA_DIST = CMakeLists.txt
