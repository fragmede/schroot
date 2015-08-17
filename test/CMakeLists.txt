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

include_directories(${GTEST_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/bin)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/schroot)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/schroot/chroot)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin-common)

set(srcdir ${PROJECT_SOURCE_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup-test-data
               ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cleanup-test-data
               ${CMAKE_CURRENT_BINARY_DIR}/cleanup-test-data)

if(BUILD_BLOCKDEV)
  set(schroot_chroot_blockdev_sources
      schroot/chroot/block-device.cc)
endif(BUILD_BLOCKDEV)

if(BUILD_LOOPBACK)
  set(schroot_chroot_loopback_sources
      schroot/chroot/loopback.cc)
endif(BUILD_LOOPBACK)

if(BUILD_BTRFSSNAP)
  set(schroot_chroot_btrfssnap_sources
      schroot/chroot/btrfs-snapshot.cc)
endif(BUILD_BTRFSSNAP)

if(BUILD_PERSONALITY)
  set(schroot_personality_sources
    schroot/personality.cc)
endif(BUILD_PERSONALITY)

set(schroot_chroot_chroot_sources
    schroot/chroot/chroot.h
    schroot/chroot/chroot.cc
    schroot/chroot/plain.cc
    schroot/chroot/custom.cc
    schroot/chroot/file.cc
    schroot/chroot/directory.cc
    ${schroot_chroot_blockdev_sources}
    ${schroot_chroot_btrfssnap_sources}
    ${schroot_chroot_loopback_sources}
    schroot/chroot/facet/userdata.cc)

if (BUILD_TESTS)
  add_test(NAME setup-test-data
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)
endif (BUILD_TESTS)

if (BUILD_TESTS)
  add_executable(schroot/chroot/chroot ${schroot_chroot_chroot_sources})
  target_link_libraries(schroot/chroot/chroot
                        libschroot
                        testmain
                        ${REGEX_LIBRARY})
  add_test(schroot/chroot/chroot schroot/chroot/chroot)
endif (BUILD_TESTS)

set(schroot_chroot_config_sources
    schroot/chroot/config.cc)

if (BUILD_TESTS)
  add_executable(schroot/chroot/config ${schroot_chroot_config_sources})
  target_link_libraries(schroot/chroot/config libschroot testmain)
  add_test(schroot/chroot/config schroot/chroot/config)
endif (BUILD_TESTS)

set(schroot_standard_sources
    schroot/environment.cc
    schroot/log.cc
    schroot/nostream.cc
    schroot/parse-value.cc
    ${schroot_personality_sources}
    schroot/regex.cc
    schroot/util.cc)

if (BUILD_TESTS)
  add_executable(schroot/standard ${schroot_standard_sources})
  target_link_libraries(schroot/standard
                        libschroot
                        ${REGEX_LIBRARY}
                        testmain)
  add_test(schroot/standard schroot/standard)
endif (BUILD_TESTS)

set(schroot_keyfile_sources
    schroot/keyfile.cc)

if (BUILD_TESTS)
  add_executable(schroot/keyfile ${schroot_keyfile_sources})
  target_link_libraries(schroot/keyfile libschroot testmain)
  add_test(schroot/keyfile schroot/keyfile)
endif (BUILD_TESTS)

set(schroot_lock_sources
    schroot/lock.cc)

if (BUILD_TESTS)
  add_executable(schroot/lock ${schroot_lock_sources})
  target_link_libraries(schroot/lock libschroot testmain)
  # lock is excluded, because it is timing dependent and can fail
  # randomly on slow or heavily-loaded systems.
endif (BUILD_TESTS)

set(schroot_run_parts_sources
    schroot/run-parts.cc)

if (BUILD_TESTS)
  add_executable(schroot/run-parts ${schroot_run_parts_sources})
  target_link_libraries(schroot/run-parts
                        libschroot
                        ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                        ${Boost_SYSTEM_LIBRARY_RELEASE}
                        testmain)
  add_test(schroot/run-parts schroot/run-parts)
endif (BUILD_TESTS)

set(bin_common_option_action_sources
    bin-common/option-action.cc)

if (BUILD_TESTS)
  add_executable(bin-common/option-action ${bin_common_option_action_sources})
  target_link_libraries(bin-common/option-action bin-common testmain)
  add_test(bin-common/option-action bin-common/option-action)
endif (BUILD_TESTS)

if (BUILD_TESTS)
  add_test(NAME cleanup-test-data
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cleanup-test-data)
endif (BUILD_TESTS)
