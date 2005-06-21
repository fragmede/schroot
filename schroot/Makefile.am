# schroot Makefile template
#
#
# Copyright (C) 2004-2005  Roger Leigh <rleigh@debian.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#####################################################################

AM_CFLAGS = -std=c99
DEFS = -DSCHROOT_CONFIG_FILE="\"$(sysconfdir)/schroot.conf\""

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
schroot_LDADD = $(GOBJECT_LIBS) $(PAM_LIBS)

sysconf_DATA = schroot.conf

EXTRA_DIST = $(sysconf_DATA)

install-exec-hook:
# Install setuid root.
	chmod 4755 $(DESTDIR)$(bindir)/schroot

