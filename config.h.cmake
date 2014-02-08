/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Set if boost::iostreams::file_descriptor close_handle is not available */
#cmakedefine BOOST_IOSTREAMS_CLOSE_HANDLE_OLD 1

/* Set if boost::program_options::options_description::options() is not
   available */
#cmakedefine BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD 1

/* Set if boost::program_options::validation error uses old construction
   semantics */
#cmakedefine BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR_OLD 1

/* Distributor name. */
#cmakedefine DISTRIBUTOR "${DISTRIBUTOR}"

/* Is distributor not set. */
#cmakedefine DISTRIBUTOR_UNSET 1

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#cmakedefine ENABLE_NLS 1

/* Define to 1 if you have the <boost/format.hpp> header file. */
#cmakedefine HAVE_BOOST_FORMAT_HPP 1

/* Define to 1 if you have the <boost/iostreams/device/file_descriptor.hpp>
   header file. */
#cmakedefine HAVE_BOOST_IOSTREAMS_DEVICE_FILE_DESCRIPTOR_HPP 1

/* Define to 1 if you have the <boost/program_options.hpp> header file. */
#cmakedefine HAVE_BOOST_PROGRAM_OPTIONS_HPP 1

/* Define to 1 if you have the <boost/type_traits.hpp> header file. */
#cmakedefine HAVE_BOOST_TYPE_TRAITS_HPP 1

/* Define to 1 if you have the <fstab.h> header file and setfstab(). */
#cmakedefine HAVE_FSTAB_FUNCTIONS 1

/* Define to 1 if you have the <mntent.h> header file and setmntent(). */
#cmakedefine HAVE_MNTENT_FUNCTIONS 1

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#cmakedefine HAVE_DCGETTEXT 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT 1

/* Define if you have the iconv() function. */
#cmakedefine HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory> header file. */
#cmakedefine HAVE_MEMORY 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine HAVE_PTHREAD 1

/* Set if the <regex> header file includes std::regex */
#cmakedefine HAVE_REGEX_REGEX 1

/* Define to 1 if you have the <sched.h> header file. */
#cmakedefine HAVE_SCHED_H 1

/* Define to 1 if you have the <security/pam_appl.h> header file. */
#cmakedefine HAVE_SECURITY_PAM_APPL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the <sys/personality.h> header file. */
#cmakedefine HAVE_SYS_PERSONALITY_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <tuple> header file. */
#cmakedefine HAVE_TUPLE 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Name of package */
#cmakedefine PACKAGE ${PACKAGE}

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT 1

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME 1

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING 1

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME 1

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL 1

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION ${VERSION}

/* Package release date (integer). */
#define RELEASE_DATE ${RELEASE_DATE}

/* Package release date (string). */
#cmakedefine RELEASE_DATE_S "${RELEASE_DATE_S}"

/* Enable debugging */
#cmakedefine SCHROOT_DEBUG 1

/* Default regular expression used to filter user environment */
#cmakedefine SCHROOT_DEFAULT_ENVIRONMENT_FILTER "${SCHROOT_DEFAULT_ENVIRONMENT_FILTER}"

/* Set if the block-device chroot type is present */
#cmakedefine SCHROOT_FEATURE_BLOCKDEV 1

/* Set if the btrfs-snapshot chroot type is present */
#cmakedefine SCHROOT_FEATURE_BTRFSSNAP 1

/* Set if the loopback chroot type is present */
#cmakedefine SCHROOT_FEATURE_LOOPBACK 1

/* Set if the lvm-snapshot chroot type is present */
#cmakedefine SCHROOT_FEATURE_LVMSNAP 1

/* Set if PAM support is available */
#cmakedefine SCHROOT_FEATURE_PAM 1

/* Set if personality support is present */
#cmakedefine SCHROOT_FEATURE_PERSONALITY 1

/* Set if the union filesystem type is present */
#cmakedefine SCHROOT_FEATURE_UNION 1

/* Set if unshare support is present */
#cmakedefine SCHROOT_FEATURE_UNSHARE 1

/* Host GNU architecture triplet */
#cmakedefine SCHROOT_HOST "${SCHROOT_HOST}"

/* Host CPU */
#cmakedefine SCHROOT_HOST_CPU "${SCHROOT_HOST_CPU}"

/* Host OS */
#cmakedefine SCHROOT_HOST_OS "${SCHROOT_HOST_OS}"

/* Host vendor */
#cmakedefine SCHROOT_HOST_VENDOR "${SCHROOT_HOST_VENDOR}"

/* Platform type, used to modify run-time platform-specific behaviour */
#cmakedefine SCHROOT_PLATFORM "${SCHROOT_PLATFORM}"

/* Test data directory */
#cmakedefine TESTDATADIR "${TESTDATADIR}"

/* Version number of package */
#cmakedefine VERSION "${VERSION}"
