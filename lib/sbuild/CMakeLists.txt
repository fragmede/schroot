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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sbuild.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/sbuild.pc @ONLY)

set(public_generated_h_sources
    config.h)

if(BUILD_PAM)
  set(public_auth_pam_h_sources
      auth/pam-conv.h
      auth/pam-conv-tty.h
      auth/pam.h
      auth/pam-message.h)
  set(public_auth_pam_cc_sources
      auth/pam-conv.cc
      auth/pam-conv-tty.cc
      auth/pam.cc
      auth/pam-message.cc)
endif(BUILD_PAM)

if(BUILD_PERSONALITY)
  set(public_personality_facet_h_sources
      chroot/facet/personality.h)
  set(public_personality_facet_cc_sources
      chroot/facet/personality.cc)
  set(public_personality_h_sources
      personality.h)
  set(public_personality_cc_sources
      personality.cc)
endif(BUILD_PERSONALITY)

if(BUILD_BLOCKDEV)
  set(public_blockdev_base_h_sources
      chroot/facet/block-device-base.h)
  set(public_blockdev_base_cc_sources
      chroot/facet/block-device-base.cc)
  set(public_blockdev_h_sources
      chroot/facet/block-device.h)
  set(public_blockdev_cc_sources
      chroot/facet/block-device.cc)
endif(BUILD_BLOCKDEV)

if(BUILD_LVMSNAP)
  set(public_blockdev_base_h_sources
      chroot/facet/block-device-base.h)
  set(public_blockdev_base_cc_sources
      chroot/facet/block-device-base.cc)
  set(public_lvmsnap_h_sources
      chroot/facet/lvm-snapshot.h)
  set(public_lvmsnap_cc_sources
      chroot/facet/lvm-snapshot.cc)
endif(BUILD_LVMSNAP)

if(BUILD_BTRFSSNAP)
  set(public_btrfssnap_h_sources
      chroot/facet/btrfs-snapshot.h)
  set(public_btrfssnap_cc_sources
      chroot/facet/btrfs-snapshot.cc)
endif(BUILD_BTRFSSNAP)

if(BUILD_LOOPBACK)
  set(public_loopback_h_sources
      chroot/facet/loopback.h)
  set(public_loopback_cc_sources
      chroot/facet/loopback.cc)
endif(BUILD_LOOPBACK)

if(BUILD_UNION)
  set(public_union_h_sources
      chroot/facet/fsunion.h)
  set(public_union_cc_sources
      chroot/facet/fsunion.cc)
endif(BUILD_UNION)

if(BUILD_UNSHARE)
  set(public_unshare_h_sources
      chroot/facet/unshare.h)
  set(public_unshare_cc_sources
      chroot/facet/unshare.cc)
endif(BUILD_UNSHARE)

set(public_h_sources
    ctty.h
    custom-error.h
    environment.h
    error.h
    error.tcc
    fdstream.h
    feature.h
    format-detail.h
    i18n.h
    keyfile.h
    keyfile-reader.h
    keyfile-writer.h
    lock.h
    log.h
    mntstream.h
    nostream.h
    parse-error.h
    parse-value.h
    regex.h
    run-parts.h
    session.h
    types.h
    util.h
    ${public_personality_h_sources})

set(public_cc_sources
    ctty.cc
    environment.cc
    feature.cc
    format-detail.cc
    keyfile.cc
    keyfile-reader.cc
    keyfile-writer.cc
    lock.cc
    log.cc
    mntstream.cc
    nostream.cc
    parse-value.cc
    run-parts.cc
    session.cc
    types.cc
    util.cc
    ${public_personality_cc_sources})

set(public_auth_h_sources
    auth/auth.h
    auth/deny.h
    ${public_auth_pam_h_sources})

set(public_auth_cc_sources
    auth/auth.cc
    auth/deny.cc
    ${public_auth_pam_cc_sources})

set(public_chroot_h_sources
    chroot/chroot.h
    chroot/config.h)

set(public_chroot_cc_sources
    chroot/chroot.cc
    chroot/config.cc)

set(public_chroot_facet_h_sources
    chroot/facet/custom.h
    chroot/facet/directory.h
    chroot/facet/directory-base.h
    chroot/facet/facet.h
    chroot/facet/factory.h
    chroot/facet/file.h
    chroot/facet/mountable.h
    chroot/facet/plain.h
    chroot/facet/session.h
    chroot/facet/session-clonable.h
    chroot/facet/session-setup.h
    chroot/facet/source.h
    chroot/facet/source-clonable.h
    chroot/facet/source-setup.h
    chroot/facet/storage.h
    chroot/facet/userdata.h
    ${public_blockdev_base_h_sources}
    ${public_blockdev_h_sources}
    ${public_lvmsnap_h_sources}
    ${public_btrfssnap_h_sources}
    ${public_loopback_h_sources}
    ${public_personality_facet_h_sources}
    ${public_union_h_sources}
    ${public_unshare_h_sources})

set(public_chroot_facet_cc_sources
    chroot/facet/custom.cc
    chroot/facet/directory.cc
    chroot/facet/directory-base.cc
    chroot/facet/facet.cc
    chroot/facet/factory.cc
    chroot/facet/file.cc
    chroot/facet/mountable.cc
    chroot/facet/plain.cc
    chroot/facet/session.cc
    chroot/facet/session-clonable.cc
    chroot/facet/session-setup.cc
    chroot/facet/source.cc
    chroot/facet/source-clonable.cc
    chroot/facet/source-setup.cc
    chroot/facet/storage.cc
    chroot/facet/userdata.cc
    ${public_blockdev_base_cc_sources}
    ${public_blockdev_cc_sources}
    ${public_lvmsnap_cc_sources}
    ${public_btrfssnap_cc_sources}
    ${public_loopback_cc_sources}
    ${public_personality_facet_cc_sources}
    ${public_union_cc_sources}
    ${public_unshare_cc_sources})

add_library(sbuild SHARED
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
                        ${REGEX_LIBRARY}
                        ${Boost_IOSTREAMS_LIBRARY_RELEASE}
                        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                        ${Boost_SYSTEM_LIBRARY_RELEASE})
set_target_properties(sbuild PROPERTIES VERSION ${GIT_RELEASE_VERSION})

set(pkgincludedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild")
set(pkgincludeauthdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild/auth")
set(pkgincludechrootdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild/chroot")
set(pkgincludechrootfacetdir "${CMAKE_INSTALL_FULL_INCLUDEDIR}/sbuild/chroot/facet")
set(pkgconfigdatadir "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

install(FILES ${public_h_sources}
        DESTINATION ${pkgincludedir})
install(FILES ${public_auth_h_sources}
        DESTINATION ${pkgincludeauthdir})
install(FILES ${public_chroot_h_sources}
        DESTINATION ${pkgincludechrootdir})
install(FILES ${public_chroot_facet_h_sources}
        DESTINATION ${pkgincludechrootfacetdir})

install(TARGETS sbuild LIBRARY
        DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

set(pkgconfig_files "${CMAKE_CURRENT_BINARY_DIR}/sbuild.pc")
install(FILES ${pkgconfig_files}
        DESTINATION ${pkgconfigdatadir})
