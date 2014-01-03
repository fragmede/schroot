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

include_directories(${PROJECT_SOURCE_DIR}/bin)

set(test_sources
    testmain.cc)

if (BUILD_TESTS)
  add_library(testmain STATIC ${test_sources})
  target_link_libraries(testmain sbuild ${CPPUNIT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
endif (BUILD_TESTS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup-test-data.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cleanup-test-data.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/cleanup-test-data)

if(BUILD_BLOCKDEV)
  set(sbuild_chroot_blockdev_sources
      sbuild-chroot-block-device.cc)
endif(BUILD_BLOCKDEV)

if(BUILD_LOOPBACK)
  set(sbuild_chroot_loopback_sources
      sbuild-chroot-loopback.cc)
endif(BUILD_LOOPBACK)

if(BUILD_LVMSNAP)
  set(sbuild_chroot_lvmsnap_sources
      sbuild-chroot-lvm-snapshot.cc)
endif(BUILD_LVMSNAP)

if(BUILD_BTRFSSNAP)
  set(sbuild_chroot_btrfssnap_sources
      sbuild-chroot-btrfs-snapshot.cc)
endif(BUILD_BTRFSSNAP)

if(BUILD_PERSONALITY)
  set(sbuild_personality_sources
    sbuild-personality.cc)
endif(BUILD_PERSONALITY)

set(sbuild_chroot_chroot_sources
    test-sbuild-chroot.h
    sbuild-chroot-plain.cc
    sbuild-chroot-custom.cc
    sbuild-chroot-file.cc
    sbuild-chroot-directory.cc
    ${sbuild_chroot_blockdev_sources}
    ${sbuild_chroot_lvmsnap_sources}
    ${sbuild_chroot_btrfssnap_sources}
    ${sbuild_chroot_loopback_sources}
    sbuild-chroot-facet-userdata.cc)

if (BUILD_TESTS)
  add_test(NAME setup-test-data
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)
endif (BUILD_TESTS)

if (BUILD_TESTS)
  add_executable(sbuild-chroot-chroot ${sbuild_chroot_chroot_sources})
  target_link_libraries(sbuild-chroot-chroot
                        sbuild
                        ${REGEX_LIBRARY}
                        testmain)
  add_test(sbuild-chroot-chroot sbuild-chroot-chroot)
endif (BUILD_TESTS)

set(sbuild_chroot_config_sources
    sbuild-chroot-config.cc)

if (BUILD_TESTS)
  add_executable(sbuild-chroot-config ${sbuild_chroot_config_sources})
  target_link_libraries(sbuild-chroot-config sbuild testmain)
  add_test(sbuild-chroot-config sbuild-chroot-config)
endif (BUILD_TESTS)

set(sbuild_standard_sources
    sbuild-environment.cc
    sbuild-log.cc
    sbuild-nostream.cc
    sbuild-parse-value.cc
    ${sbuild_personality_sources}
    sbuild-regex.cc
    sbuild-util.cc)

if (BUILD_TESTS)
  add_executable(sbuild-standard ${sbuild_standard_sources})
  target_link_libraries(sbuild-standard
                        sbuild
                        ${REGEX_LIBRARY}
                        testmain)
  add_test(sbuild-standard sbuild-standard)
endif (BUILD_TESTS)

set(sbuild_keyfile_sources
    sbuild-keyfile.cc)

if (BUILD_TESTS)
  add_executable(sbuild-keyfile ${sbuild_keyfile_sources})
  target_link_libraries(sbuild-keyfile sbuild testmain)
  add_test(sbuild-keyfile sbuild-keyfile)
endif (BUILD_TESTS)

set(sbuild_lock_sources
    sbuild-lock.cc)

if (BUILD_TESTS)
  add_executable(sbuild-lock ${sbuild_lock_sources})
  target_link_libraries(sbuild-lock sbuild testmain)
  # lock is excluded, because it is timing dependent and can fail
  # randomly on slow or heavily-loaded systems.
endif (BUILD_TESTS)

set(sbuild_run_parts_sources
    sbuild-run-parts.cc)

if (BUILD_TESTS)
  add_executable(sbuild-run-parts ${sbuild_run_parts_sources})
  target_link_libraries(sbuild-run-parts
                        sbuild
                        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                        ${Boost_SYSTEM_LIBRARY_RELEASE}
                        testmain)
  add_test(sbuild-run-parts sbuild-run-parts)
endif (BUILD_TESTS)

set(schroot_base_option_action_sources
    schroot-base-option-action.cc)

if (BUILD_TESTS)
  add_executable(schroot-base-option-action ${schroot_base_option_action_sources})
  target_link_libraries(schroot-base-option-action schroot-base testmain)
  add_test(schroot-base-option-action schroot-base-option-action)
endif (BUILD_TESTS)

if (BUILD_TESTS)
  add_test(NAME cleanup-test-data
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cleanup-test-data)
endif (BUILD_TESTS)
