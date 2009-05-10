# schroot Makefile template
#
#
# Copyright © 2004-2008  Roger Leigh <rleigh@debian.org>
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

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS)

DEFS = -D_GNU_SOURCE

lib_LTLIBRARIES = libsbuild.la

sbuild_public_h_sources =		\
	sbuild-auth.h			\
	sbuild-auth-null.h		\
	sbuild-chroot.h			\
	sbuild-chroot-block-device.h	\
	sbuild-chroot-directory.h	\
	sbuild-chroot-file.h		\
	sbuild-chroot-mountable.h	\
	sbuild-chroot-plain.h		\
	sbuild-chroot-source.h		\
	sbuild-chroot-config.h		\
	sbuild-ctty.h			\
	sbuild-custom-error.h		\
	sbuild-dirstream.h		\
	sbuild-environment.h		\
	sbuild-error.h			\
	sbuild-error.tcc		\
	sbuild-format-detail.h		\
	sbuild-i18n.h			\
	sbuild-keyfile-base.h		\
	sbuild-basic-keyfile.h		\
	sbuild-basic-keyfile.tcc	\
	sbuild-keyfile.h		\
	sbuild-lock.h			\
	sbuild-log.h			\
	sbuild-mntstream.h		\
	sbuild-nostream.h		\
	sbuild-null.h			\
	sbuild-parse-error.h		\
	sbuild-parse-value.h		\
	sbuild-personality.h		\
	sbuild-regex.h			\
	sbuild-run-parts.h		\
	sbuild-session.h		\
	sbuild-types.h			\
	sbuild-tr1types.h		\
	sbuild-util.h

if BUILD_PAM
sbuild_public_auth_h_sources =		\
	sbuild-auth-conv.h		\
	sbuild-auth-conv-tty.h		\
	sbuild-auth-pam.h		\
	sbuild-auth-message.h
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_h_sources =	\
	sbuild-chroot-block-device.h
endif

if BUILD_LVMSNAP
sbuild_public_lvmsnap_h_sources =	\
	sbuild-chroot-lvm-snapshot.h
endif

if BUILD_LOOPBACK
sbuild_public_loopback_h_sources =	\
	sbuild-chroot-loopback.h
endif

sbuild_public_cc_sources =		\
	sbuild-auth.cc			\
	sbuild-auth-null.cc		\
	sbuild-chroot.cc		\
	sbuild-chroot-directory.cc	\
	sbuild-chroot-file.cc		\
	sbuild-chroot-mountable.cc	\
	sbuild-chroot-plain.cc		\
	sbuild-chroot-source.cc		\
	sbuild-chroot-config.cc		\
	sbuild-ctty.cc			\
	sbuild-dirstream.cc		\
	sbuild-environment.cc		\
	sbuild-format-detail.cc		\
	sbuild-keyfile-base.cc		\
	sbuild-lock.cc			\
	sbuild-log.cc			\
	sbuild-mntstream.cc		\
	sbuild-nostream.cc		\
	sbuild-null.cc			\
	sbuild-parse-value.cc		\
	sbuild-personality.cc		\
	sbuild-run-parts.cc		\
	sbuild-session.cc		\
	sbuild-types.cc			\
	sbuild-util.cc

if BUILD_PAM
sbuild_public_auth_cc_sources =	\
	sbuild-auth-conv.cc		\
	sbuild-auth-conv-tty.cc		\
	sbuild-auth-pam.cc		\
	sbuild-auth-message.cc
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_cc_sources =	\
	sbuild-chroot-block-device.cc
endif

if BUILD_LVMSNAP
sbuild_public_lvmsnap_cc_sources =	\
	sbuild-chroot-lvm-snapshot.cc
endif

if BUILD_LOOPBACK
sbuild_public_loopback_cc_sources =	\
	sbuild-chroot-loopback.cc
endif

pkgincludedir = $(includedir)/sbuild

pkginclude_HEADERS =				\
	$(sbuild_public_h_sources)		\
	$(sbuild_public_blockdev_h_sources)

libsbuild_la_SOURCES =				\
	$(sbuild_public_h_sources)		\
	$(sbuild_public_cc_sources)		\
	$(sbuild_public_auth_h_sources)		\
	$(sbuild_public_auth_cc_sources)	\
	$(sbuild_public_blockdev_h_sources)	\
	$(sbuild_public_blockdev_cc_sources)	\
	$(sbuild_public_lvmsnap_h_sources)	\
	$(sbuild_public_lvmsnap_cc_sources)	\
	$(sbuild_public_loopback_h_sources)	\
	$(sbuild_public_loopback_cc_sources)

nodist_libsbuild_la_SOURCES =	\
	sbuild-config.h

libsbuild_la_LIBADD = $(UUID_LIBS) $(PAM_LIBS) $(LOCKDEV_LIBS) $(BOOST_LIBS) $(LIBINTL)

pkgconfigdatadir = $(libdir)/pkgconfig

pkgconfigdata_DATA = sbuild.pc

# Uncommment to build an unversioned library (version in soname)
#libsbuild_version = -release $(SBUILD_VERSION)
# Uncomment to build a versioned library
libsbuild_version = -version-info $(SBUILD_CURRENT_INTERFACE):$(SBUILD_INTERFACE_AGE):$(SBUILD_BINARY_AGE)
libsbuild_la_LDFLAGS = $(libsbuild_version) \
        -rpath $(libdir) -no-undefined
