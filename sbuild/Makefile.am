# schroot Makefile template
#
#
# Copyright © 2004-2009  Roger Leigh <rleigh@debian.org>
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

lib_LTLIBRARIES = libsbuild.la

sbuild_public_h_sources =			\
	sbuild-auth.h				\
	sbuild-auth-null.h			\
	sbuild-chroot.h				\
	sbuild-chroot-custom.h			\
	sbuild-chroot-directory.h		\
	sbuild-chroot-directory-base.h		\
	sbuild-chroot-facet.h			\
	sbuild-chroot-facet-mountable.h		\
	sbuild-chroot-facet-personality.h	\
	sbuild-chroot-facet-session.h		\
	sbuild-chroot-facet-session-clonable.h	\
	sbuild-chroot-facet-source.h		\
	sbuild-chroot-facet-source-clonable.h	\
	sbuild-chroot-facet-userdata.h		\
	sbuild-chroot-file.h			\
	sbuild-chroot-plain.h			\
	sbuild-chroot-config.h			\
	sbuild-ctty.h				\
	sbuild-custom-error.h			\
	sbuild-environment.h			\
	sbuild-error.h				\
	sbuild-error.tcc			\
	sbuild-fdstream.h			\
	sbuild-feature.h			\
	sbuild-format-detail.h			\
	sbuild-i18n.h				\
	sbuild-keyfile-base.h			\
	sbuild-basic-keyfile.h			\
	sbuild-basic-keyfile.tcc		\
	sbuild-keyfile.h			\
	sbuild-lock.h				\
	sbuild-log.h				\
	sbuild-mntstream.h			\
	sbuild-nostream.h			\
	sbuild-null.h				\
	sbuild-parse-error.h			\
	sbuild-parse-value.h			\
	sbuild-personality.h			\
	sbuild-regex.h				\
	sbuild-run-parts.h			\
	sbuild-session.h			\
	sbuild-types.h				\
	sbuild-tr1types.h			\
	sbuild-util.h

if BUILD_PAM
sbuild_public_auth_pam_h_sources =	\
	sbuild-auth-pam-conv.h		\
	sbuild-auth-pam-conv-tty.h	\
	sbuild-auth-pam.h		\
	sbuild-auth-pam-message.h
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_base_h_sources =	\
	sbuild-chroot-block-device-base.h
sbuild_public_blockdev_h_sources =	\
	sbuild-chroot-block-device.h
endif

if BUILD_LVMSNAP
sbuild_public_blockdev_base_h_sources =	\
	sbuild-chroot-block-device-base.h
sbuild_public_lvmsnap_h_sources =	\
	sbuild-chroot-lvm-snapshot.h
endif

if BUILD_BTRFSSNAP
sbuild_public_btrfssnap_h_sources =	\
	sbuild-chroot-btrfs-snapshot.h
endif

if BUILD_LOOPBACK
sbuild_public_loopback_h_sources =	\
	sbuild-chroot-loopback.h
endif

if BUILD_UNION
sbuild_public_union_h_sources =	\
	sbuild-chroot-facet-union.h
endif

sbuild_public_cc_sources =			\
	sbuild-auth.cc				\
	sbuild-auth-null.cc			\
	sbuild-chroot.cc			\
	sbuild-chroot-custom.cc			\
	sbuild-chroot-directory.cc		\
	sbuild-chroot-directory-base.cc		\
	sbuild-chroot-facet-mountable.cc	\
	sbuild-chroot-facet-personality.cc	\
	sbuild-chroot-facet-session.cc		\
	sbuild-chroot-facet-session-clonable.cc	\
	sbuild-chroot-facet-source.cc		\
	sbuild-chroot-facet-source-clonable.cc	\
	sbuild-chroot-facet-userdata.cc		\
	sbuild-chroot-file.cc			\
	sbuild-chroot-plain.cc			\
	sbuild-chroot-config.cc			\
	sbuild-ctty.cc				\
	sbuild-environment.cc			\
	sbuild-feature.cc			\
	sbuild-format-detail.cc			\
	sbuild-keyfile-base.cc			\
	sbuild-lock.cc				\
	sbuild-log.cc				\
	sbuild-mntstream.cc			\
	sbuild-nostream.cc			\
	sbuild-null.cc				\
	sbuild-parse-value.cc			\
	sbuild-personality.cc			\
	sbuild-run-parts.cc			\
	sbuild-session.cc			\
	sbuild-types.cc				\
	sbuild-util.cc

if BUILD_PAM
sbuild_public_auth_pam_cc_sources =	\
	sbuild-auth-pam-conv.cc		\
	sbuild-auth-pam-conv-tty.cc	\
	sbuild-auth-pam.cc		\
	sbuild-auth-pam-message.cc
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_base_cc_sources =	\
	sbuild-chroot-block-device-base.cc
sbuild_public_blockdev_cc_sources =	\
	sbuild-chroot-block-device.cc
endif

if BUILD_LVMSNAP
sbuild_public_blockdev_base_cc_sources =	\
	sbuild-chroot-block-device-base.cc
sbuild_public_lvmsnap_cc_sources =	\
	sbuild-chroot-lvm-snapshot.cc
endif

if BUILD_BTRFSSNAP
sbuild_public_btrfssnap_cc_sources =	\
	sbuild-chroot-btrfs-snapshot.cc
endif

if BUILD_LOOPBACK
sbuild_public_loopback_cc_sources =	\
	sbuild-chroot-loopback.cc
endif

if BUILD_UNION
sbuild_public_union_cc_sources =	\
	sbuild-chroot-facet-union.cc
endif

pkgincludedir = $(includedir)/sbuild

pkginclude_HEADERS =					\
	$(sbuild_public_h_sources)			\
	$(sbuild_public_blockdev_base_h_sources)	\
	$(sbuild_public_blockdev_h_sources)		\
	$(sbuild_public_lvmsnap_h_sources)		\
	$(sbuild_public_btrfssnap_h_sources)		\
	$(sbuild_public_loopback_h_sources)		\
	$(sbuild_public_union_h_sources)

libsbuild_la_SOURCES =					\
	$(sbuild_public_h_sources)			\
	$(sbuild_public_cc_sources)			\
	$(sbuild_public_auth_pam_h_sources)		\
	$(sbuild_public_auth_pam_cc_sources)		\
	$(sbuild_public_blockdev_base_h_sources)	\
	$(sbuild_public_blockdev_base_cc_sources)	\
	$(sbuild_public_blockdev_h_sources)		\
	$(sbuild_public_blockdev_cc_sources)		\
	$(sbuild_public_lvmsnap_h_sources)		\
	$(sbuild_public_lvmsnap_cc_sources)		\
	$(sbuild_public_btrfssnap_h_sources)		\
	$(sbuild_public_btrfssnap_cc_sources)		\
	$(sbuild_public_loopback_h_sources)		\
	$(sbuild_public_loopback_cc_sources)		\
	$(sbuild_public_union_h_sources)		\
	$(sbuild_public_union_cc_sources)

nodist_libsbuild_la_SOURCES =	\
	sbuild-config.h

libsbuild_la_LIBADD = $(UUID_LIBS) $(PAM_LIBS) $(BOOST_LIBS) $(BOOST_FILESYSTEM_LIBS) $(LIBINTL)

pkgconfigdatadir = $(libdir)/pkgconfig

pkgconfigdata_DATA = sbuild.pc

# Uncommment to build an unversioned library (version in soname)
#libsbuild_version = -release $(SBUILD_VERSION)
# Uncomment to build a versioned library
libsbuild_version = -version-info $(SBUILD_CURRENT_INTERFACE):$(SBUILD_INTERFACE_AGE):$(SBUILD_BINARY_AGE)
libsbuild_la_LDFLAGS = $(libsbuild_version) \
        -rpath $(libdir) -no-undefined

EXTRA_DIST =			\
	CMakeLists.txt		\
	sbuild-config.h.cmake	\
	sbuild.pc.cmake
