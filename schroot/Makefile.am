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

SUBDIRS = pam

localedir = $(datadir)/locale

AM_CFLAGS = -std=c99
DEFS = -DSCHROOT_CONFIG_FILE="\"$(sysconfdir)/schroot.conf\"" -DG_LOG_DOMAIN=\"Schroot\" -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = schroot

schroot_SOURCES = \
	schroot.c \
	sbuild-chroot.h \
	sbuild-chroot.c \
	sbuild-config.h \
	sbuild-config.c \
	sbuild-session.h \
	sbuild-session.c

schroot_CFLAGS = $(AM_CFLAGS) $(SCHROOT_CFLAGS)
schroot_LDADD = $(GOBJECT_LIBS) $(PAM_LIBS) $(LIBINTL)

if ENABLE_GTK_DOC
noinst_LTLIBRARIES = libschroot.la
libschroot_la_SOURCES = $(schroot_SOURCES)
libschroot_la_CFLAGS = $(AM_CFLAGS) $(SCHROOT_CFLAGS)
libschroot_la_LIBADD = $(GOBJECT_LIBS) $(PAM_LIBS)
endif

sysconf_DATA = schroot.conf

man_MANS = schroot.1 schroot.conf.5

EXTRA_DIST = $(sysconf_DATA)

install-exec-hook:
# Install setuid root.
	chmod 4755 $(DESTDIR)$(bindir)/schroot

