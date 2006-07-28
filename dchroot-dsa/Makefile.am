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

include $(top_srcdir)/scripts/global.mk

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS)

DEFS = -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

if BUILD_DCHROOT_DSA
dchroot_dsa = dchroot-dsa
endif

bin_PROGRAMS = $(dchroot_dsa)

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
dchroot_dsa_LDADD = $(top_builddir)/dchroot/libdchroot.la

if BUILD_DCHROOT_DSA
dchroot_dsa_mans = dchroot-dsa.1
endif

man_MANS = $(dchroot_dsa_mans)

install-exec-hook:
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/dchroot-dsa" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/dchroot-dsa"; \
	fi
