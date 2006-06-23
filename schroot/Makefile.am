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

SUBDIRS = pam exec setup

localedir = $(datadir)/locale
pkglibexecdir = $(SCHROOT_LIBEXEC_DIR)

AM_CXXFLAGS = $(SCHROOT_CFLAGS) -pedantic -Wall -Wcast-align -Wwrite-strings -Wswitch-default -Wcast-qual -Wunused-variable -Wredundant-decls -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast -Woverloaded-virtual -fstrict-aliasing
#  -Weffc++ causes too many warnings in standard headers; -Wextra is not
#  supported by GCC 3.4.

DEFS = -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

noinst_LTLIBRARIES = libschroot.la

bin_PROGRAMS = schroot
pkglibexec_PROGRAMS = schroot-releaselock schroot-listmounts

libschroot_la_SOURCES =			\
	schroot-options-base.h		\
	schroot-options-base.cc		\
	schroot-main.h			\
	schroot-main.cc
libschroot_la_LIBADD = $(top_builddir)/sbuild/libsbuild.la

schroot_SOURCES =			\
	schroot-options.h		\
	schroot-options.cc		\
	schroot.cc
schroot_LDADD = libschroot.la

schroot_releaselock_SOURCES = 		\
	schroot-releaselock-options.h	\
	schroot-releaselock-options.cc	\
	schroot-releaselock.cc
schroot_releaselock_LDADD = $(top_builddir)/sbuild/libsbuild.la

schroot_listmounts_SOURCES =		\
	schroot-listmounts-options.h	\
	schroot-listmounts-options.cc	\
	schroot-listmounts.cc
schroot_listmounts_LDADD = $(top_builddir)/sbuild/libsbuild.la


pkgsysconfdir = $(PACKAGE_SYSCONF_DIR)
pkgsysconf_DATA = schroot.conf

man_MANS = schroot.1 schroot-setup.5 schroot.conf.5

EXTRA_DIST =			\
	$(pkgsysconf_DATA)

install-exec-hook:
# Install setuid root.
	chmod 4755 "$(DESTDIR)$(bindir)/schroot"

install-data-hook:
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_MOUNT_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_SESSION_DIR)
