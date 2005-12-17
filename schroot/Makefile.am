# schroot Makefile template
#
#
# Copyright © 2004-2005  Roger Leigh <rleigh@debian.org>
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

AM_CFLAGS = -std=c99 $(SCHROOT_CFLAGS)
AM_CXXFLAGS = $(SCHROOT_CFLAGS)

DEFS = -DG_LOG_DOMAIN=\"Schroot\" -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

bin_PROGRAMS = schroot
pkglibexec_PROGRAMS = schroot-releaselock

schroot_public_h_sources =		\
	sbuild-auth.h			\
	sbuild-auth-conv.h		\
	sbuild-auth-conv-tty.h		\
	sbuild-auth-message.h   	\
	sbuild-chroot.h			\
	sbuild-chroot-block-device.h	\
	sbuild-chroot-lvm-snapshot.h	\
	sbuild-chroot-plain.h		\
	sbuild-config.h			\
	sbuild-error.h			\
	sbuild-keyfile.h		\
	sbuild-lock.h			\
	sbuild-session.h		\
	sbuild-util.h

schroot_public_cc_sources =		\
	sbuild-auth.cc			\
	sbuild-auth-conv.cc		\
	sbuild-auth-conv-tty.cc		\
	sbuild-auth-message.cc		\
	sbuild-chroot.cc		\
	sbuild-chroot-block-device.cc	\
	sbuild-chroot-lvm-snapshot.cc	\
	sbuild-chroot-plain.cc		\
	sbuild-config.cc		\
	sbuild-error.cc			\
	sbuild-lock.cc			\
	sbuild-session.cc		\
	sbuild-util.cc

schroot_SOURCES =			\
	$(schroot_public_h_sources)	\
	$(schroot_public_cc_sources)	\
	schroot-options.h		\
	schroot-options.cc		\
	schroot.h			\
	schroot.cc

schroot_LDADD = $(GOBJECT_LIBS) $(UUID_LIBS) $(SIGC_LIBS) $(PAM_LIBS) $(LOCKDEV_LIBS) $(LIBINTL)

#if ENABLE_GTK_DOC
#noinst_LTLIBRARIES = libschroot.la
#libschroot_la_SOURCES = $(schroot_SOURCES)
#libschroot_la_CFLAGS = $(schroot_CFLAGS)
#libschroot_la_LIBADD = $(schroot_LDADD)
#endif

schroot_releaselock_SOURCES = \
	schroot-releaselock.cc
schroot_releaselock_CFLAGS = $(AM_CFLAGS) $(SCHROOT_CFLAGS)
schroot_releaselock_LDADD = $(GOBJECT_LIBS) $(LOCKDEV_LIBS) $(LIBINTL)

pkgsysconfdir = $(PACKAGE_SYSCONF_DIR)
pkgsysconf_DATA = schroot.conf

man_MANS = schroot.1 schroot-setup.5 schroot.conf.5

EXTRA_DIST =				\
	$(pkgsysconf_DATA)

install-exec-hook:
# Install setuid root.
	chmod 4755 $(DESTDIR)$(bindir)/schroot

install-data-hook:
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_LOCK_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_MOUNT_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_SESSION_DIR)
