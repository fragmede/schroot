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

SUBDIRS = pam run setup

localedir = $(datadir)/locale
pkglibexecdir = $(SCHROOT_LIBEXEC_DIR)

AM_CXXFLAGS = $(SCHROOT_CFLAGS) -pedantic -Wall -Wcast-align -Wextra -Wwrite-strings -Wswitch-default -Wcast-qual -Wunused-variable -Wredundant-decls -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast -Woverloaded-virtual -fstrict-aliasing
#  -Weffc++ causes too many warnings in standard headers.

DEFS = -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

lib_LTLIBRARIES = libsbuild.la
bin_PROGRAMS = schroot
pkglibexec_PROGRAMS = schroot-releaselock

BUILT_SOURCES = sbuild.gch

sbuild.gch: $(sbuild_public_h_sources)
	$(CXXCOMPILE) -o $@ -c sbuild.h

sbuild_public_h_sources =		\
	sbuild.h			\
	sbuild-auth.h			\
	sbuild-auth-conv.h		\
	sbuild-auth-conv-tty.h		\
	sbuild-auth-message.h   	\
	sbuild-chroot.h			\
	sbuild-chroot-block-device.h	\
	sbuild-chroot-file.h		\
	sbuild-chroot-lvm-snapshot.h	\
	sbuild-chroot-plain.h		\
	sbuild-config.h			\
	sbuild-environment.h		\
	sbuild-error.h			\
	sbuild-i18n.h			\
	sbuild-keyfile.h		\
	sbuild-lock.h			\
	sbuild-log.h			\
	sbuild-nostream.h		\
	sbuild-parse-value.h		\
	sbuild-session.h		\
	sbuild-types.h			\
	sbuild-util.h

sbuild_public_cc_sources =		\
	sbuild-auth.cc			\
	sbuild-auth-conv.cc		\
	sbuild-auth-conv-tty.cc		\
	sbuild-auth-message.cc		\
	sbuild-chroot.cc		\
	sbuild-chroot-block-device.cc	\
	sbuild-chroot-file.cc		\
	sbuild-chroot-lvm-snapshot.cc	\
	sbuild-chroot-plain.cc		\
	sbuild-config.cc		\
	sbuild-environment.cc		\
	sbuild-keyfile.cc		\
	sbuild-lock.cc			\
	sbuild-log.cc			\
	sbuild-nostream.cc		\
	sbuild-parse-value.cc		\
	sbuild-session.cc		\
	sbuild-util.cc

libsbuild_la_SOURCES =			\
	$(sbuild_public_h_sources)	\
	$(sbuild_public_cc_sources)

libsbuild_la_LIBADD = $(UUID_LIBS) $(PAM_LIBS) $(LOCKDEV_LIBS) $(BOOST_LIBS) $(LIBINTL)

schroot_SOURCES =			\
	schroot-options.h		\
	schroot-options.cc		\
	schroot.cc

schroot_LDADD = libsbuild.la

schroot_releaselock_SOURCES = 		\
	schroot-releaselock-options.h	\
	schroot-releaselock-options.cc	\
	schroot-releaselock.cc
schroot_releaselock_CFLAGS = $(AM_CFLAGS) $(SCHROOT_CFLAGS)
schroot_releaselock_LDADD = libsbuild.la

pkgsysconfdir = $(PACKAGE_SYSCONF_DIR)
pkgsysconf_DATA = schroot.conf

man_MANS = schroot.1 schroot-setup.5 schroot.conf.5

CLEANFILES = sbuild.gch

EXTRA_DIST =				\
	$(pkgsysconf_DATA)

install-exec-hook:
# Install setuid root.
	chmod 4755 $(DESTDIR)$(bindir)/schroot

install-data-hook:
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_LOCK_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_MOUNT_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_SESSION_DIR)
