# Copyright © 2004-2013  Roger Leigh <rleigh@debian.org>
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

lib_LTLIBRARIES = sbuild/libsbuild.la

sbuild_public_h_sources =				\
	sbuild/sbuild-auth.h				\
	sbuild/sbuild-auth-null.h			\
	sbuild/sbuild-chroot.h				\
	sbuild/sbuild-chroot-custom.h			\
	sbuild/sbuild-chroot-directory.h		\
	sbuild/sbuild-chroot-directory-base.h		\
	sbuild/sbuild-chroot-facet.h			\
	sbuild/sbuild-chroot-facet-mountable.h		\
	sbuild/sbuild-chroot-facet-personality.h	\
	sbuild/sbuild-chroot-facet-session.h		\
	sbuild/sbuild-chroot-facet-session-clonable.h	\
	sbuild/sbuild-chroot-facet-source.h		\
	sbuild/sbuild-chroot-facet-source-clonable.h	\
	sbuild/sbuild-chroot-facet-userdata.h		\
	sbuild/sbuild-chroot-file.h			\
	sbuild/sbuild-chroot-plain.h			\
	sbuild/sbuild-chroot-config.h			\
	sbuild/sbuild-ctty.h				\
	sbuild/sbuild-custom-error.h			\
	sbuild/sbuild-environment.h			\
	sbuild/sbuild-error.h				\
	sbuild/sbuild-error.tcc				\
	sbuild/sbuild-fdstream.h			\
	sbuild/sbuild-feature.h				\
	sbuild/sbuild-format-detail.h			\
	sbuild/sbuild-i18n.h				\
	sbuild/sbuild-keyfile.h				\
	sbuild/sbuild-keyfile-reader.h			\
	sbuild/sbuild-keyfile-writer.h			\
	sbuild/sbuild-lock.h				\
	sbuild/sbuild-log.h				\
	sbuild/sbuild-mntstream.h			\
	sbuild/sbuild-nostream.h			\
	sbuild/sbuild-parse-error.h			\
	sbuild/sbuild-parse-value.h			\
	sbuild/sbuild-personality.h			\
	sbuild/sbuild-regex.h				\
	sbuild/sbuild-run-parts.h			\
	sbuild/sbuild-session.h				\
	sbuild/sbuild-types.h				\
	sbuild/sbuild-util.h

if BUILD_PAM
sbuild_public_auth_pam_h_sources =		\
	sbuild/sbuild-auth-pam-conv.h		\
	sbuild/sbuild-auth-pam-conv-tty.h	\
	sbuild/sbuild-auth-pam.h		\
	sbuild/sbuild-auth-pam-message.h
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_base_h_sources =	\
	sbuild/sbuild-chroot-block-device-base.h
sbuild_public_blockdev_h_sources =	\
	sbuild/sbuild-chroot-block-device.h
endif

if BUILD_LVMSNAP
sbuild_public_blockdev_base_h_sources =	\
	sbuild/sbuild-chroot-block-device-base.h
sbuild_public_lvmsnap_h_sources =	\
	sbuild/sbuild-chroot-lvm-snapshot.h
endif

if BUILD_BTRFSSNAP
sbuild_public_btrfssnap_h_sources =	\
	sbuild/sbuild-chroot-btrfs-snapshot.h
endif

if BUILD_LOOPBACK
sbuild_public_loopback_h_sources =	\
	sbuild/sbuild-chroot-loopback.h
endif

if BUILD_UNION
sbuild_public_union_h_sources =	\
	sbuild/sbuild-chroot-facet-union.h
endif

if BUILD_UNSHARE
sbuild_public_unshare_h_sources =	\
	sbuild/sbuild-chroot-facet-unshare.h
endif

sbuild_public_cc_sources =				\
	sbuild/sbuild-auth.cc				\
	sbuild/sbuild-auth-null.cc			\
	sbuild/sbuild-chroot.cc				\
	sbuild/sbuild-chroot-custom.cc			\
	sbuild/sbuild-chroot-directory.cc		\
	sbuild/sbuild-chroot-directory-base.cc		\
	sbuild/sbuild-chroot-facet-mountable.cc		\
	sbuild/sbuild-chroot-facet-personality.cc	\
	sbuild/sbuild-chroot-facet-session.cc		\
	sbuild/sbuild-chroot-facet-session-clonable.cc	\
	sbuild/sbuild-chroot-facet-source.cc		\
	sbuild/sbuild-chroot-facet-source-clonable.cc	\
	sbuild/sbuild-chroot-facet-userdata.cc		\
	sbuild/sbuild-chroot-file.cc			\
	sbuild/sbuild-chroot-plain.cc			\
	sbuild/sbuild-chroot-config.cc			\
	sbuild/sbuild-ctty.cc				\
	sbuild/sbuild-environment.cc			\
	sbuild/sbuild-feature.cc			\
	sbuild/sbuild-format-detail.cc			\
	sbuild/sbuild-keyfile.cc			\
	sbuild/sbuild-keyfile-reader.cc			\
	sbuild/sbuild-keyfile-writer.cc			\
	sbuild/sbuild-lock.cc				\
	sbuild/sbuild-log.cc				\
	sbuild/sbuild-mntstream.cc			\
	sbuild/sbuild-nostream.cc			\
	sbuild/sbuild-parse-value.cc			\
	sbuild/sbuild-personality.cc			\
	sbuild/sbuild-run-parts.cc			\
	sbuild/sbuild-session.cc			\
	sbuild/sbuild-types.cc				\
	sbuild/sbuild-util.cc

if BUILD_PAM
sbuild_public_auth_pam_cc_sources =		\
	sbuild/sbuild-auth-pam-conv.cc		\
	sbuild/sbuild-auth-pam-conv-tty.cc	\
	sbuild/sbuild-auth-pam.cc		\
	sbuild/sbuild-auth-pam-message.cc
endif

if BUILD_BLOCKDEV
sbuild_public_blockdev_base_cc_sources =	\
	sbuild/sbuild-chroot-block-device-base.cc
sbuild_public_blockdev_cc_sources =	\
	sbuild/sbuild-chroot-block-device.cc
endif

if BUILD_LVMSNAP
sbuild_public_blockdev_base_cc_sources =	\
	sbuild/sbuild-chroot-block-device-base.cc
sbuild_public_lvmsnap_cc_sources =	\
	sbuild/sbuild-chroot-lvm-snapshot.cc
endif

if BUILD_BTRFSSNAP
sbuild_public_btrfssnap_cc_sources =	\
	sbuild/sbuild-chroot-btrfs-snapshot.cc
endif

if BUILD_LOOPBACK
sbuild_public_loopback_cc_sources =	\
	sbuild/sbuild-chroot-loopback.cc
endif

if BUILD_UNION
sbuild_public_union_cc_sources =	\
	sbuild/sbuild-chroot-facet-union.cc
endif

if BUILD_UNSHARE
sbuild_public_unshare_cc_sources =	\
	sbuild/sbuild-chroot-facet-unshare.cc
endif

pkgincludedir = $(includedir)/sbuild

pkginclude_HEADERS =					\
	$(sbuild_public_h_sources)			\
	$(sbuild_public_blockdev_base_h_sources)	\
	$(sbuild_public_blockdev_h_sources)		\
	$(sbuild_public_lvmsnap_h_sources)		\
	$(sbuild_public_btrfssnap_h_sources)		\
	$(sbuild_public_loopback_h_sources)		\
	$(sbuild_public_union_h_sources)		\
	$(sbuild_public_unshare_h_sources)

sbuild_libsbuild_la_SOURCES =				\
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
	$(sbuild_public_union_cc_sources)		\
	$(sbuild_public_unshare_h_sources)		\
	$(sbuild_public_unshare_cc_sources)

nodist_sbuild_libsbuild_la_SOURCES =	\
	sbuild/sbuild-config.h

sbuild_libsbuild_la_LIBADD = $(UUID_LIBS) $(PAM_LIBS) $(BOOST_LIBS) $(BOOST_FILESYSTEM_LIBS) $(LIBINTL)

pkgconfigdatadir = $(libdir)/pkgconfig

pkgconfigdata_DATA = sbuild/sbuild.pc

# Uncommment to build an unversioned library (version in soname)
#sbuild_libsbuild_version = -release $(SBUILD_VERSION)
# Uncomment to build a versioned library
sbuild_libsbuild_version = -version-info $(SBUILD_CURRENT_INTERFACE):$(SBUILD_INTERFACE_AGE):$(SBUILD_BINARY_AGE)
sbuild_libsbuild_la_LDFLAGS = $(libsbuild_version) \
        -rpath $(libdir) -no-undefined
