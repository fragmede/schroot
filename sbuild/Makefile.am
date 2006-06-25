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

localedir = $(datadir)/locale

AM_CXXFLAGS = $(SCHROOT_CFLAGS) -pedantic -Wall -Wcast-align -Wwrite-strings -Wswitch-default -Wcast-qual -Wunused-variable -Wredundant-decls -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast -Woverloaded-virtual -fstrict-aliasing
#  -Weffc++ causes too many warnings in standard headers; -Wextra is not
#  supported by GCC 3.4.

DEFS = -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\" -D_GNU_SOURCE

noinst_LTLIBRARIES = libsbuild.la

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
	sbuild-tr1types.h		\
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
