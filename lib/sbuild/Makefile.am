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

lib_LTLIBRARIES = lib/sbuild/libsbuild.la

lib_sbuild_public_h_sources =				\
	lib/sbuild/auth.h				\
	lib/sbuild/auth-null.h				\
	lib/sbuild/chroot/chroot.h			\
	lib/sbuild/chroot/custom.h			\
	lib/sbuild/chroot/directory.h			\
	lib/sbuild/chroot/directory-base.h		\
	lib/sbuild/chroot/facet/facet.h			\
	lib/sbuild/chroot/facet/mountable.h		\
	lib/sbuild/chroot/facet/personality.h		\
	lib/sbuild/chroot/facet/session.h		\
	lib/sbuild/chroot/facet/session-clonable.h	\
	lib/sbuild/chroot/facet/source.h		\
	lib/sbuild/chroot/facet/source-clonable.h	\
	lib/sbuild/chroot-facet-userdata.h		\
	lib/sbuild/chroot/file.h			\
	lib/sbuild/chroot/plain.h			\
	lib/sbuild/chroot/config.h			\
	lib/sbuild/ctty.h				\
	lib/sbuild/custom-error.h			\
	lib/sbuild/environment.h			\
	lib/sbuild/error.h				\
	lib/sbuild/error.tcc				\
	lib/sbuild/fdstream.h				\
	lib/sbuild/feature.h				\
	lib/sbuild/format-detail.h			\
	lib/sbuild/i18n.h				\
	lib/sbuild/keyfile.h				\
	lib/sbuild/keyfile-reader.h			\
	lib/sbuild/keyfile-writer.h			\
	lib/sbuild/lock.h				\
	lib/sbuild/log.h				\
	lib/sbuild/mntstream.h				\
	lib/sbuild/nostream.h				\
	lib/sbuild/parse-error.h			\
	lib/sbuild/parse-value.h			\
	lib/sbuild/personality.h			\
	lib/sbuild/regex.h				\
	lib/sbuild/run-parts.h				\
	lib/sbuild/session.h				\
	lib/sbuild/types.h				\
	lib/sbuild/util.h

if BUILD_PAM
lib_sbuild_public_auth_pam_h_sources =	\
	lib/sbuild/auth-pam-conv.h	\
	lib/sbuild/auth-pam-conv-tty.h	\
	lib/sbuild/auth-pam.h		\
	lib/sbuild/auth-pam-message.h
endif

if BUILD_BLOCKDEV
lib_sbuild_public_blockdev_base_h_sources =	\
	lib/sbuild/chroot/block-device-base.h
lib_sbuild_public_blockdev_h_sources =	\
	lib/sbuild/chroot/block-device.h
endif

if BUILD_LVMSNAP
lib_sbuild_public_blockdev_base_h_sources =	\
	lib/sbuild/chroot/block-device-base.h
lib_sbuild_public_lvmsnap_h_sources =	\
	lib/sbuild/chroot/lvm-snapshot.h
endif

if BUILD_BTRFSSNAP
lib_sbuild_public_btrfssnap_h_sources =	\
	lib/sbuild/chroot/btrfs-snapshot.h
endif

if BUILD_LOOPBACK
lib_sbuild_public_loopback_h_sources =	\
	lib/sbuild/chroot/loopback.h
endif

if BUILD_UNION
lib_sbuild_public_union_h_sources =	\
	lib/sbuild/chroot/facet/fsunion.h
endif

if BUILD_UNSHARE
lib_sbuild_public_unshare_h_sources =	\
	lib/sbuild/chroot/facet/unshare.h
endif

lib_sbuild_public_cc_sources =				\
	lib/sbuild/auth.cc				\
	lib/sbuild/auth-null.cc				\
	lib/sbuild/chroot/chroot.cc			\
	lib/sbuild/chroot/custom.cc			\
	lib/sbuild/chroot/directory.cc			\
	lib/sbuild/chroot/directory-base.cc		\
	lib/sbuild/chroot/facet/mountable.cc		\
	lib/sbuild/chroot/facet/personality.cc		\
	lib/sbuild/chroot/facet/session.cc		\
	lib/sbuild/chroot/facet/session-clonable.cc	\
	lib/sbuild/chroot/facet/source.cc		\
	lib/sbuild/chroot/facet/source-clonable.cc	\
	lib/sbuild/chroot-facet-userdata.cc		\
	lib/sbuild/chroot/file.cc			\
	lib/sbuild/chroot/plain.cc			\
	lib/sbuild/chroot/config.cc			\
	lib/sbuild/ctty.cc				\
	lib/sbuild/environment.cc			\
	lib/sbuild/feature.cc				\
	lib/sbuild/format-detail.cc			\
	lib/sbuild/keyfile.cc				\
	lib/sbuild/keyfile-reader.cc			\
	lib/sbuild/keyfile-writer.cc			\
	lib/sbuild/lock.cc				\
	lib/sbuild/log.cc				\
	lib/sbuild/mntstream.cc				\
	lib/sbuild/nostream.cc				\
	lib/sbuild/parse-value.cc			\
	lib/sbuild/personality.cc			\
	lib/sbuild/run-parts.cc				\
	lib/sbuild/session.cc				\
	lib/sbuild/types.cc				\
	lib/sbuild/util.cc

if BUILD_PAM
lib_sbuild_public_auth_pam_cc_sources =		\
	lib/sbuild/auth-pam-conv.cc		\
	lib/sbuild/auth-pam-conv-tty.cc		\
	lib/sbuild/auth-pam.cc			\
	lib/sbuild/auth-pam-message.cc
endif

if BUILD_BLOCKDEV
lib_sbuild_public_blockdev_base_cc_sources =	\
	lib/sbuild/chroot/block-device-base.cc
lib_sbuild_public_blockdev_cc_sources =	\
	lib/sbuild/chroot/block-device.cc
endif

if BUILD_LVMSNAP
lib_sbuild_public_blockdev_base_cc_sources =	\
	lib/sbuild/chroot/block-device-base.cc
lib_sbuild_public_lvmsnap_cc_sources =	\
	lib/sbuild/chroot/lvm-snapshot.cc
endif

if BUILD_BTRFSSNAP
lib_sbuild_public_btrfssnap_cc_sources =	\
	lib/sbuild/chroot/btrfs-snapshot.cc
endif

if BUILD_LOOPBACK
lib_sbuild_public_loopback_cc_sources =	\
	lib/sbuild/chroot/loopback.cc
endif

if BUILD_UNION
lib_sbuild_public_union_cc_sources =	\
	lib/sbuild/chroot/facet/fsunion.cc
endif

if BUILD_UNSHARE
lib_sbuild_public_unshare_cc_sources =	\
	lib/sbuild/chroot/facet/unshare.cc
endif

pkgincludedir = $(includedir)/sbuild

pkginclude_HEADERS =					\
	$(lib_sbuild_public_h_sources)			\
	$(lib_sbuild_public_blockdev_base_h_sources)	\
	$(lib_sbuild_public_blockdev_h_sources)		\
	$(lib_sbuild_public_lvmsnap_h_sources)		\
	$(lib_sbuild_public_btrfssnap_h_sources)	\
	$(lib_sbuild_public_loopback_h_sources)		\
	$(lib_sbuild_public_union_h_sources)		\
	$(lib_sbuild_public_unshare_h_sources)

lib_sbuild_libsbuild_la_SOURCES =			\
	$(lib_sbuild_public_h_sources)			\
	$(lib_sbuild_public_cc_sources)			\
	$(lib_sbuild_public_auth_pam_h_sources)		\
	$(lib_sbuild_public_auth_pam_cc_sources)	\
	$(lib_sbuild_public_blockdev_base_h_sources)	\
	$(lib_sbuild_public_blockdev_base_cc_sources)	\
	$(lib_sbuild_public_blockdev_h_sources)		\
	$(lib_sbuild_public_blockdev_cc_sources)	\
	$(lib_sbuild_public_lvmsnap_h_sources)		\
	$(lib_sbuild_public_lvmsnap_cc_sources)		\
	$(lib_sbuild_public_btrfssnap_h_sources)	\
	$(lib_sbuild_public_btrfssnap_cc_sources)	\
	$(lib_sbuild_public_loopback_h_sources)		\
	$(lib_sbuild_public_loopback_cc_sources)	\
	$(lib_sbuild_public_union_h_sources)		\
	$(lib_sbuild_public_union_cc_sources)		\
	$(lib_sbuild_public_unshare_h_sources)		\
	$(lib_sbuild_public_unshare_cc_sources)

nodist_lib_sbuild_libsbuild_la_SOURCES =	\
	lib/sbuild/config.h

lib_sbuild_libsbuild_la_LIBADD = $(PAM_LIBS) $(BOOST_LIBS) $(BOOST_FILESYSTEM_LIBS) $(LIBINTL)

pkgconfigdatadir = $(libdir)/pkgconfig

pkgconfigdata_DATA = lib/sbuild/sbuild.pc

# Uncommment to build an unversioned library (version in soname)
#lib_sbuild_libsbuild_version = -release $(SBUILD_VERSION)
# Uncomment to build a versioned library
lib_sbuild_libsbuild_version = -version-info $(SBUILD_CURRENT_INTERFACE):$(SBUILD_INTERFACE_AGE):$(SBUILD_BINARY_AGE)
lib_sbuild_libsbuild_la_LDFLAGS = $(libsbuild_version) \
        -rpath $(libdir) -no-undefined
