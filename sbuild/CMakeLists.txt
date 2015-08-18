# Copyright © 2004-2013  Roger Leigh <rleigh@codelibre.net>
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sbuild-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/sbuild-config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sbuild.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/sbuild.pc @ONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(public_generated_h_sources
    config.h)

if(BUILD_PAM)
  set(public_auth_pam_h_sources
      sbuild-auth-pam-conv.h
      sbuild-auth-pam-conv-tty.h
      sbuild-auth-pam.h
      sbuild-auth-pam-message.h)
  set(public_auth_pam_cc_sources
      sbuild-auth-pam-conv.cc
      sbuild-auth-pam-conv-tty.cc
      sbuild-auth-pam.cc
      sbuild-auth-pam-message.cc)
endif(BUILD_PAM)

if(BUILD_BLOCKDEV)
  set(public_blockdev_base_h_sources
      sbuild-chroot-block-device-base.h)
  set(public_blockdev_base_cc_sources
      sbuild-chroot-block-device-base.cc)
  set(public_blockdev_h_sources
      sbuild-chroot-block-device.h)
  set(public_blockdev_cc_sources
      sbuild-chroot-block-device.cc)
endif(BUILD_BLOCKDEV)

if(BUILD_LVMSNAP)
  set(public_blockdev_base_h_sources
      sbuild-chroot-block-device-base.h)
  set(public_blockdev_base_cc_sources
      sbuild-chroot-block-device-base.cc)
  set(public_lvmsnap_h_sources
      sbuild-chroot-lvm-snapshot.h)
  set(public_lvmsnap_cc_sources
      sbuild-chroot-lvm-snapshot.cc)
endif(BUILD_LVMSNAP)

if(BUILD_BTRFSSNAP)
  set(public_btrfssnap_h_sources
      sbuild-chroot-btrfs-snapshot.h)
  set(public_btrfssnap_cc_sources
      sbuild-chroot-btrfs-snapshot.cc)
endif(BUILD_BTRFSSNAP)

if(BUILD_LOOPBACK)
  set(public_loopback_h_sources
      sbuild-chroot-loopback.h)
  set(public_loopback_cc_sources
      sbuild-chroot-loopback.cc)
endif(BUILD_LOOPBACK)

if(BUILD_UNION)
  set(public_union_h_sources
      sbuild-chroot-facet-union.h)
  set(public_union_cc_sources
      sbuild-chroot-facet-union.cc)
endif(BUILD_UNION)

set(public_h_sources
    sbuild-basic-keyfile.h
    sbuild-basic-keyfile.tcc
    sbuild-ctty.h
    sbuild-custom-error.h
    sbuild-environment.h
    sbuild-error.h
    sbuild-error.tcc
    sbuild-fdstream.h
    sbuild-feature.h
    sbuild-format-detail.h
    sbuild-i18n.h
    sbuild-keyfile-base.h
    sbuild-keyfile.h
    sbuild-lock.h
    sbuild-log.h
    sbuild-mntstream.h
    sbuild-nostream.h
    sbuild-null.h
    sbuild-parse-error.h
    sbuild-parse-value.h
    sbuild-personality.h
    sbuild-regex.h
    sbuild-run-parts.h
    sbuild-session.h
    sbuild-tr1types.h
    sbuild-types.h
    sbuild-util.h)

set(public_cc_sources
    sbuild-ctty.cc
    sbuild-environment.cc
    sbuild-feature.cc
    sbuild-format-detail.cc
    sbuild-keyfile-base.cc
    sbuild-lock.cc
    sbuild-log.cc
    sbuild-mntstream.cc
    sbuild-nostream.cc
    sbuild-null.cc
    sbuild-parse-value.cc
    sbuild-personality.cc
    sbuild-run-parts.cc
    sbuild-session.cc
    sbuild-types.cc
    sbuild-util.cc)

set(public_auth_h_sources
    sbuild-auth.h
    sbuild-auth-null.h
    ${public_auth_pam_h_sources})

set(public_auth_cc_sources
    sbuild-auth.cc
    sbuild-auth-null.cc
    ${public_auth_pam_cc_sources})

set(public_chroot_h_sources
    sbuild-chroot.h
    sbuild-chroot-config.h
    sbuild-chroot-custom.h
    sbuild-chroot-directory.h
    sbuild-chroot-directory-base.h
    sbuild-chroot-file.h
    sbuild-chroot-plain.h
    ${public_blockdev_base_h_sources}
    ${public_blockdev_h_sources}
    ${public_lvmsnap_h_sources}
    ${public_btrfssnap_h_sources}
    ${public_loopback_h_sources})

set(public_chroot_cc_sources
    sbuild-chroot.cc
    sbuild-chroot-config.cc
    sbuild-chroot-custom.cc
    sbuild-chroot-directory.cc
    sbuild-chroot-directory-base.cc
    sbuild-chroot-file.cc
    sbuild-chroot-plain.cc
    ${public_blockdev_base_cc_sources}
    ${public_blockdev_cc_sources}
    ${public_lvmsnap_cc_sources}
    ${public_btrfssnap_cc_sources}
    ${public_loopback_cc_sources})

set(public_chroot_facet_h_sources
    sbuild-chroot-facet.h
    sbuild-chroot-facet-mountable.h
    sbuild-chroot-facet-personality.h
    sbuild-chroot-facet-session.h
    sbuild-chroot-facet-session-clonable.h
    sbuild-chroot-facet-source.h
    sbuild-chroot-facet-source-clonable.h
    sbuild-chroot-facet-userdata.h
    ${public_union_h_sources}
    ${public_unshare_h_sources})

set(public_chroot_facet_cc_sources
    sbuild-chroot-facet-mountable.cc
    sbuild-chroot-facet-personality.cc
    sbuild-chroot-facet-session.cc
    sbuild-chroot-facet-session-clonable.cc
    sbuild-chroot-facet-source.cc
    sbuild-chroot-facet-source-clonable.cc
    sbuild-chroot-facet-userdata.cc
    ${public_union_cc_sources}
    ${public_unshare_cc_sources})

add_library(sbuild STATIC
            ${public_h_sources}
            ${public_cc_sources}
            ${public_auth_h_sources}
            ${public_auth_cc_sources}
            ${public_chroot_h_sources}
            ${public_chroot_cc_sources}
            ${public_chroot_facet_h_sources}
            ${public_chroot_facet_cc_sources})
target_link_libraries(sbuild
                      PRIVATE
                        ${CMAKE_THREAD_LIBS_INIT}
                        ${PAM_LIBRARY}
                        ${UUID_LIBRARY}
                        ${REGEX_LIBRARY}
                        ${Boost_IOSTREAMS_LIBRARY_RELEASE}
                        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                        ${Boost_SYSTEM_LIBRARY_RELEASE})

set(pkgincludedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild")
set(pkgincludeauthdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild")
set(pkgincludechrootdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild")
set(pkgincludechrootfacetdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild")
set(pkgconfigdatadir "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

install(FILES ${public_h_sources}
        DESTINATION ${pkgincludedir})
install(FILES ${public_auth_h_sources}
        DESTINATION ${pkgincludeauthdir})
install(FILES ${public_chroot_h_sources}
        DESTINATION ${pkgincludechrootdir})
install(FILES ${public_chroot_facet_h_sources}
        DESTINATION ${pkgincludechrootfacetdir})

install(TARGETS sbuild ARCHIVE
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

set(pkgconfig_files "${CMAKE_CURRENT_BINARY_DIR}/sbuild.pc")
install(FILES ${pkgconfig_files}
        DESTINATION ${pkgconfigdatadir})
