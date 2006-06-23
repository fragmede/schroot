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

noinst_LTLIBRARIES = libsbuild.la libschroot.la libdchroot.la

if BUILD_DCHROOT
dchroot = dchroot
endif

if BUILD_DCHROOT_DSA
dchroot_dsa = dchroot-dsa
endif

bin_PROGRAMS = $(dchroot) $(dchroot_dsa) schroot
pkglibexec_PROGRAMS = schroot-releaselock schroot-listmounts

sbuild_public_h_sources =		\
	sbuild-auth.h			\
	sbuild-auth-conv.h		\
	sbuild-auth-conv-tty.h		\
	sbuild-auth-message.h   	\
	sbuild-chroot.h			\
	sbuild-chroot-block-device.h	\
	sbuild-chroot-file.h		\
	sbuild-chroot-lvm-snapshot.h	\
	sbuild-chroot-plain.h		\
	sbuild-chroot-source.h		\
	sbuild-chroot-config.h		\
	sbuild-custom-error.h		\
	sbuild-custom-error.tcc		\
	sbuild-environment.h		\
	sbuild-error.h			\
	sbuild-format-detail.h		\
	sbuild-i18n.h			\
	sbuild-keyfile.h		\
	sbuild-lock.h			\
	sbuild-log.h			\
	sbuild-nostream.h		\
	sbuild-parse-error.h		\
	sbuild-parse-value.h		\
	sbuild-personality.h		\
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
	sbuild-chroot-source.cc		\
	sbuild-chroot-config.cc		\
	sbuild-environment.cc		\
	sbuild-format-detail.cc		\
	sbuild-keyfile.cc		\
	sbuild-lock.cc			\
	sbuild-log.cc			\
	sbuild-nostream.cc		\
	sbuild-parse-error.cc		\
	sbuild-parse-value.cc		\
	sbuild-personality.cc		\
	sbuild-session.cc		\
	sbuild-util.cc

libsbuild_la_SOURCES =			\
	$(sbuild_public_h_sources)	\
	$(sbuild_public_cc_sources)

nodist_libsbuild_la_SOURCES =	\
	sbuild-config.h

libsbuild_la_LIBADD = $(UUID_LIBS) $(PAM_LIBS) $(LOCKDEV_LIBS) $(BOOST_LIBS) $(LIBINTL)

libschroot_la_SOURCES =			\
	schroot-options-base.h		\
	schroot-options-base.cc		\
	schroot-main.h			\
	schroot-main.cc
libschroot_la_LIBADD = libsbuild.la

libdchroot_la_SOURCES =			\
	dchroot-session-base.h		\
	dchroot-session-base.cc
libdchroot_la_LIBADD = libschroot.la

schroot_SOURCES =			\
	schroot-options.h		\
	schroot-options.cc		\
	schroot.cc
schroot_LDADD = libschroot.la

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

dchroot_dsa_SOURCES =			\
	dchroot-dsa-chroot-config.h	\
	dchroot-dsa-chroot-config.cc	\
	dchroot-dsa-session.h		\
	dchroot-dsa-session.cc		\
	dchroot-dsa-options.h		\
	dchroot-dsa-options.cc		\
	dchroot-dsa-main.h		\
	dchroot-dsa-main.cc		\
	dchroot-dsa.cc
dchroot_dsa_LDADD = libdchroot.la

schroot_releaselock_SOURCES = 		\
	schroot-releaselock-options.h	\
	schroot-releaselock-options.cc	\
	schroot-releaselock.cc
schroot_releaselock_LDADD = libsbuild.la

schroot_listmounts_SOURCES =		\
	schroot-listmounts-options.h	\
	schroot-listmounts-options.cc	\
	schroot-listmounts.cc
schroot_listmounts_LDADD = libsbuild.la


pkgsysconfdir = $(PACKAGE_SYSCONF_DIR)
pkgsysconf_DATA = schroot.conf

if BUILD_DCHROOT
dchroot_mans = dchroot.1
endif

if BUILD_DCHROOT_DSA
dchroot_dsa_mans = dchroot-dsa.1
endif

man_MANS = $(dchroot_mans) $(dchroot_dsa_mans) schroot.1 schroot-setup.5 schroot.conf.5

EXTRA_DIST =			\
	$(pkgsysconf_DATA)

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/dchroot" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot"; \
	fi
	if [ -f "$(DESTDIR)$(bindir)/dchroot-dsa" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot-dsa"; \
	fi
	chmod 4755 "$(DESTDIR)$(bindir)/schroot"

install-data-hook:
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_MOUNT_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_SESSION_DIR)
