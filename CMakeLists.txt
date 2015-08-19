# Copyright © 2004-2013  Roger Leigh <rleigh@codelibre.net>
#
# schroot is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# schroot is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8.12)
cmake_policy(VERSION 2.8.12)

project(schroot)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
  message(FATAL_ERROR "In-tree builds are not supported; please run cmake from a separate build directory.")
endif("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")

# Obtain version information from VERSION and/or git.
set(GIT_VERSION_FILE "cmake/GitVersion.cmake"
    CACHE FILEPATH "Location of version metadata")
set(GIT_RELEASE_POLICY_FILE "${PROJECT_SOURCE_DIR}/cmake/GitReleasePolicy.cmake"
    CACHE FILEPATH "Location of release policy configuration")
include("${PROJECT_SOURCE_DIR}/cmake/GitRelease.cmake")

set(VERSION ${GIT_RELEASE_VERSION})
set(RELEASE_DATE ${GIT_RELEASE_DATE_UNIX})
set(RELEASE_DATE_S ${GIT_RELEASE_DATE})

string(REGEX MATCH "^([0-9]+-[0-9]+-[0-9]+).*" date_valid "${RELEASE_DATE_S}")
if (date_valid)
  string(REGEX REPLACE "^([0-9]+-[0-9]+-[0-9]+).*" "\\1"
         RELEASE_DATE_SHORT ${RELEASE_DATE_S})
else (date_valid)
  set(RELEASE_DATE_SHORT ${RELEASE_DATE_S})
endif (date_valid)

message(STATUS "Configuring ${CMAKE_PROJECT_NAME} ${GIT_RELEASE_VERSION} (${RELEASE_DATE_SHORT})")

# TODO: Check NEWS version

include(GNUInstallDirs)
include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include("cmake/CompilerChecks.cmake")

find_package(Threads REQUIRED)
find_package(GTest)

include(FindBoost)
find_package(Boost REQUIRED
             COMPONENTS filesystem system iostreams program_options regex)

# HEADER CHECKS
include(CheckIncludeFileCXX)

# LIBRARY CHECKS
include (CheckLibraryExists)
include (CheckFunctionExists)
include (CheckSymbolExists)

include("cmake/BoostChecks.cmake")
include("cmake/RegexChecks.cmake")

# Configure debugging
option(debug "Enable debugging messages" OFF)
set(SCHROOT_DEBUG 0)
if(debug)
  set(SCHROOT_DEBUG 1)
endif(debug)

# Configure debugging
set(BUILD_TESTS OFF)
if(GTEST_FOUND)
  set(BUILD_TESTS ON)
endif(GTEST_FOUND)
option(test "Enable unit tests" ${BUILD_TESTS})
set(BUILD_TESTS ${test})

# Environment filter default
set(default_environment_filter "^(BASH_ENV|CDPATH|ENV|HOSTALIASES|IFS|KRB5_CONFIG|KRBCONFDIR|KRBTKFILE|KRB_CONF|LD_.*|LOCALDOMAIN|NLSPATH|PATH_LOCALE|RES_OPTIONS|TERMINFO|TERMINFO_DIRS|TERMPATH)\$"
    CACHE STRING "Default environment filter")
set(SCHROOT_DEFAULT_ENVIRONMENT_FILTER ${default_environment_filter})

# Distributor
set(distributor "Unknown"
    CACHE STRING "Distributor of this build")
set(DISTRIBUTOR "${distributor}")
if(distributor STREQUAL Unknown)
  set(DISTRIBUTOR_UNSET ON)
endif(distributor STREQUAL Unknown)

# bash completion directory
set(bash_completion_dir "${CMAKE_INSTALL_SYSCONFDIR}/bash_completion.d"
    CACHE PATH "bash shell completion directory")
#==> bashcompletiondir (use cache val directly)

# schroot directories
set(SCHROOT_LOCALE_DIR "${CMAKE_INSTALL_FULL_LOCALEDIR}"
    CACHE PATH "Locale directory")
set(SCHROOT_MOUNT_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${CMAKE_PROJECT_NAME}/mount"
    CACHE PATH "Directory under which mount chroots")
set(SCHROOT_SESSION_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${CMAKE_PROJECT_NAME}/session"
    CACHE PATH "Directory for storing session metadata")
set(SCHROOT_FILE_UNPACK_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${CMAKE_PROJECT_NAME}/unpack"
    CACHE PATH "Directory for unpacking chroot file archives under")
set(SCHROOT_OVERLAY_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${CMAKE_PROJECT_NAME}/union/overlay"
    CACHE PATH "Directory for union filesystem writable overlays")
set(SCHROOT_UNDERLAY_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${CMAKE_PROJECT_NAME}/union/underlay"
    CACHE PATH "Directory for union filesystem read-only underlays")
set(SCHROOT_MODULE_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_PROJECT_NAME}/${GIT_RELEASE_VERSION}/modules"
    CACHE PATH "Directory for loadable modules")
set(SCHROOT_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "Directory for schroot data files")
set(SCHROOT_LIBEXEC_DIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "Directory for schroot helper programs")
set(SCHROOT_SYSCONF_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "Directory for configuration files")
set(SCHROOT_CONF_CHROOT_D "${SCHROOT_SYSCONF_DIR}/chroot.d"
    CACHE PATH "Directory for chroot configuration fragments")
set(SCHROOT_CONF_SETUP_D "${SCHROOT_SYSCONF_DIR}/setup.d"
    CACHE PATH "Directory for chroot setup scripts")
set(SCHROOT_SETUP_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/setup"
    CACHE PATH "Directory for common setup script data")
mark_as_advanced(SCHROOT_LOCALE_DIR SCHROOT_MOUNT_DIR
                 SCHROOT_SESSION_DIR SCHROOT_FILE_UNPACK_DIR
                 SCHROOT_OVERLAY_DIR SCHROOT_UNDERLAY_DIR
                 SCHROOT_MODULE_DIR SCHROOT_DATA_DIR
                 SCHROOT_LIBEXEC_DIR SCHROOT_SYSCONF_DIR
                 SCHROOT_CONF_CHROOT_D SCHROOT_CONF_SETUP_D
                 SCHROOT_SETUP_DATA_DIR)

set(TESTDATADIR "${PROJECT_BINARY_DIR}/test/testdata")

# schroot files
set(SCHROOT_CONF "${SCHROOT_SYSCONF_DIR}/schroot.conf")

# Platform
string(TOLOWER ${CMAKE_SYSTEM_NAME} SCHROOT_PLATFORM)

# Global environ
check_symbol_exists(environ unistd.h HAVE_UNISTD_ENVIRON)

# Localisation with gettext
include(FindGettext)
find_package(Gettext)
set(NLS_DEFAULT OFF)
if(GETTEXT_FOUND)
  set(NLS_DEFAULT ON)
endif(GETTEXT_FOUND)
option(nls "Enable national language support (requires gettext)" ${NLS_DEFAULT})
set(BUILD_NLS ${nls})
set(SCHROOT_FEATURE_NLS ${pam})

# PAM authentication feature
check_include_file_cxx (security/pam_appl.h PAM_HEADER)
check_library_exists(pam pam_authenticate "" PAM_FUNC)
check_include_file_cxx (security/openpam.h OPENPAM_HEADER)
check_library_exists(pam openpam_ttyconv "" OPENPAM_FUNC)
check_include_file_cxx (security/pam_misc.h LINUXPAM_MISC_HEADER)
check_library_exists(pam misc_conv "" LINUXPAM_MISC_FUNC)
set(PAM_DEFAULT OFF)
if(PAM_HEADER AND PAM_FUNC)
  set(PAM_DEFAULT ON)
endif(PAM_HEADER AND PAM_FUNC)
option(pam "Enable support for PAM authentication (requires libpam)" ${PAM_DEFAULT})
set(BUILD_PAM ${pam})
set(SCHROOT_FEATURE_PAM ${pam})
if(BUILD_PAM)
  set(PAM_LIBRARY pam)
endif(BUILD_PAM)

# Set early, so it can be overridden by block-device
set(BLOCKDEV_DEFAULT ON)

# Btrfs snapshot mount feature
find_program(BTRFS_EXECUTABLE btrfs PATHS /sbin /usr/sbin /usr/local/sbin)
set(BTRFSSNAP_DEFAULT OFF)
if (BTRFS_EXECUTABLE)
  set (BTRFSSNAP_DEFAULT ON)
endif (BTRFS_EXECUTABLE)
option(btrfs-snapshot "Enable support for btrfs snapshots (requires Btrfs)" ${BTRFSSNAP_DEFAULT})
set(BUILD_BTRFSSNAP ${btrfs-snapshot})
set(SCHROOT_FEATURE_BTRFSSNAP ${btrfs-snapshot})
if (btrfs-snapshot)
  set(BLOCKDEV_DEFAULT ON)
endif(btrfs-snapshot)

# Block device mount feature
option(block-device "Enable support for block devices" ${BLOCKDEV_DEFAULT})
set(BUILD_BLOCKDEV ${block-device})
set(SCHROOT_FEATURE_BLOCKDEV ${block-device})
if(btrfs-snapshot AND NOT block-device)
  message(FATAL_ERROR "block-device must be enabled when btrfs-snapshot is enabled")
endif(btrfs-snapshot AND NOT block-device)

# Loopback mount feature
find_program(LOSETUP_EXECUTABLE losetup PATHS /sbin /usr/sbin /usr/local/sbin)
set(LOOPBACK_DEFAULT OFF)
if (LOSETUP_EXECUTABLE)
  set (LOOPBACK_DEFAULT ON)
endif (LOSETUP_EXECUTABLE)
option(loopback "Enable support for loopback mounts" ${LOOPBACK_DEFAULT})
set(BUILD_LOOPBACK ${loopback})
set(SCHROOT_FEATURE_LOOPBACK ${loopback})

# Filesystem union mount feature
set(UNION_DEFAULT ON)
option(union "Enable support for union mounts" ${UNION_DEFAULT})
set(BUILD_UNION ${union})
set(SCHROOT_FEATURE_UNION ${union})

# Doxygen documentation
include(FindDoxygen)
find_package(Doxygen)
set(DOXYGEN_DEFAULT OFF)
if (DOXYGEN_EXECUTABLE)
  set (DOXYGEN_DEFAULT ON)
endif (DOXYGEN_EXECUTABLE)
option(doxygen "Enable doxygen documentation" ${DOXYGEN_DEFAULT})
set(BUILD_DOXYGEN ${doxygen})

# Linux and BSD fstab parsing
check_include_file_cxx(fstab.h FSTAB_HEADER)
check_function_exists(setfstab FSTAB_FUNC)
if (FSTAB_HEADER AND FSTAB_FUNC)
  set(HAVE_FSTAB_FUNCTIONS TRUE)
else (FSTAB_HEADER AND FSTAB_FUNC)
  check_include_file_cxx(mntent.h MNTENT_HEADER)
  check_function_exists(setmntent MNTENT_FUNC)
  if (MNTENT_HEADER AND MNTENT_FUNC)
    set(HAVE_MNTENT_FUNCTIONS TRUE)
  else (MNTENT_HEADER AND MNTENT_FUNC)
    message(FATAL_ERROR "No fstab parser available for this platform.")
  endif (MNTENT_HEADER AND MNTENT_FUNC)
endif (FSTAB_HEADER AND FSTAB_FUNC)

# Namespace unshare feature
# sched.h ==> UNSHARE_HEADER
check_include_file_cxx (sched.h UNSHARE_HEADER)
check_function_exists(unshare UNSHARE_FUNC)
set(UNSHARE_DEFAULT OFF)
if (UNSHARE_HEADER AND UNSHARE_FUNC)
  set (UNSHARE_DEFAULT ON)
endif (UNSHARE_HEADER AND UNSHARE_FUNC)
option(unshare "Enable unshare support (Linux only)" ${UNSHARE_DEFAULT})
set(BUILD_UNSHARE ${unshare})
set(SCHROOT_FEATURE_UNSHARE ${unshare})

# Kernel personality feature
# sys/personality.h ==> PERSONALITY_HEADER
check_include_file_cxx (sys/personality.h PERSONALITY_HEADER)
check_function_exists(personality PERSONALITY_FUNC)
set(PERSONALITY_DEFAULT OFF)
if (PERSONALITY_HEADER AND PERSONALITY_FUNC)
  set (PERSONALITY_DEFAULT ON)
endif (PERSONALITY_HEADER AND PERSONALITY_FUNC)
option(personality "Enable personality support (Linux only)" ${PERSONALITY_DEFAULT})
set(BUILD_PERSONALITY ${personality})
set(SCHROOT_FEATURE_PERSONALITY ${personality})

# GENERATED FILES:
configure_file(${PROJECT_SOURCE_DIR}/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)

include_directories(${PROJECT_BINARY_DIR}/lib
                    ${PROJECT_SOURCE_DIR}/lib
                    ${PROJECT_BINARY_DIR}
                    ${PROJECT_SOURCE_DIR})

enable_testing()

add_subdirectory(lib/schroot)
add_subdirectory(lib/bin-common)
add_subdirectory(lib/schroot-common)
add_subdirectory(lib/test)
add_subdirectory(bin/schroot)
add_subdirectory(libexec/listmounts)
add_subdirectory(libexec/mount)
add_subdirectory(test)
add_subdirectory(doc)
add_subdirectory(etc)
add_subdirectory(po)
add_subdirectory(man)

add_custom_target(check-news
  COMMAND if grep -q "Welcome to ${CMAKE_PROJECT_NAME} ${GIT_RELEASE_VERSION}" "${PROJECT_SOURCE_DIR}/NEWS" \; then echo "NEWS current version ${GIT_RELEASE_VERSION}" \; else echo "NEWS current version does not match release version ${GIT_RELEASE_VERSION}" \; exit 1 \; fi
  COMMAND if grep -q "Major changes in ${GIT_RELEASE_VERSION}:" "${PROJECT_SOURCE_DIR}/NEWS" \; then echo "NEWS contains changes for version ${GIT_RELEASE_VERSION}" \; else echo "NEWS version does not contain changes for version ${GIT_RELEASE_VERSION}" \; exit 1 \; fi)

if(GIT_RELEASE_ENABLE)
  add_dependencies(git-release check-news)
endif(GIT_RELEASE_ENABLE)

if(GIT_DIST_ENABLE)
  add_custom_target(git-changelog
                    COMMAND git log > ${GIT_DIST_ROOT}/ChangeLog
                    DEPENDS git-distdir-archive
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  add_dependencies(git-distdir git-changelog)
endif(GIT_DIST_ENABLE)
