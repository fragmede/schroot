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

localedir = $(datadir)/locale

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) -I$(top_srcdir)/bin

LOCAL_DEFS = -DLOCALEDIR=\"$(localedir)\"

noinst_LTLIBRARIES = libschroot-base.la

libschroot_base_la_SOURCES =		\
	schroot-base-main.h		\
	schroot-base-main.cc		\
	schroot-base-option-action.h	\
	schroot-base-option-action.cc	\
	schroot-base-options.h		\
	schroot-base-options.cc		\
	schroot-base-run.h
libschroot_base_la_LIBADD = $(top_builddir)/sbuild/libsbuild.la

EXTRA_DIST = CMakeLists.txt
