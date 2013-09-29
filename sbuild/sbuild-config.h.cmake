/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
 *
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#ifndef SBUILD_CONFIG_H
#define SBUILD_CONFIG_H

/* This header contains configuration macros which determine the
   correct library to use.  This depends upon the libraries found at
   configure time. */

/* Set if device locking support is available */
#cmakedefine SBUILD_FEATURE_DEVLOCK 1

/* Set if NLS support is available */
#cmakedefine SBUILD_FEATURE_NLS 1

/* Set if PAM support is available */
#cmakedefine SBUILD_FEATURE_PAM 1

/* Set if personality support is present */
#cmakedefine SBUILD_FEATURE_PERSONALITY 1

/* Set if the block-device chroot type is present */
#cmakedefine SBUILD_FEATURE_BLOCKDEV 1

/* Set if the btrfs-snapshot chroot type is present */
#cmakedefine SBUILD_FEATURE_BTRFSSNAP 1

/* Set if the lvm-snapshot chroot type is present */
#cmakedefine SBUILD_FEATURE_LVMSNAP 1

/* Set if the loopback chroot type is present */
#cmakedefine SBUILD_FEATURE_LOOPBACK 1

/* Set if the union filesystem type is present */
#cmakedefine SBUILD_FEATURE_UNION 1

/* Define to 1 if you have the <boost/format.hpp> header file. */
#cmakedefine HAVE_BOOST_FORMAT_HPP 1

/* Define to 1 if you have the <boost/program_options.hpp> header file. */
#cmakedefine HAVE_BOOST_PROGRAM_OPTIONS_HPP 1

/* Define to 1 if you have the <memory> header file. */
#cmakedefine HAVE_MEMORY_SHARED_PTR 1

/* Define to 1 if you have the <tr1/memory> header file. */
#cmakedefine HAVE_TR1_MEMORY 1

/* Define to 1 if you have the <tuple> header file. */
#cmakedefine HAVE_TUPLE 1

/* Define to 1 if you have the <tr1/tuple> header file. */
#cmakedefine HAVE_TR1_TUPLE 1

/* Define to 1 if you have the <regex> header file and std::regex. */
#cmakedefine HAVE_REGEX_REGEX 1

/* Default regular expression used to filter user environment */
#cmakedefine SBUILD_DEFAULT_ENVIRONMENT_FILTER "${SBUILD_DEFAULT_ENVIRONMENT_FILTER}"

/* Filesystem locations */
#cmakedefine SCHROOT_LIBEXEC_DIR "${SCHROOT_LIBEXEC_DIR}"
#cmakedefine SCHROOT_MOUNT_DIR "${SCHROOT_MOUNT_DIR}"
#cmakedefine SCHROOT_SESSION_DIR "${SCHROOT_SESSION_DIR}"
#cmakedefine SCHROOT_FILE_UNPACK_DIR "${SCHROOT_FILE_UNPACK_DIR}"
#cmakedefine SCHROOT_OVERLAY_DIR "${SCHROOT_OVERLAY_DIR}"
#cmakedefine SCHROOT_UNDERLAY_DIR "${SCHROOT_UNDERLAY_DIR}"
#cmakedefine SCHROOT_SYSCONF_DIR "${SCHROOT_SYSCONF_DIR}"
#cmakedefine SCHROOT_CONF "${SCHROOT_CONF}"
#cmakedefine SCHROOT_CONF_CHROOT_D "${SCHROOT_CONF_CHROOT_D}"
#cmakedefine SCHROOT_CONF_SETUP_D "${SCHROOT_CONF_SETUP_D}"
#cmakedefine SCHROOT_SETUP_DATA_DIR "${SCHROOT_SETUP_DATA_DIR}"
#cmakedefine SCHROOT_LOCALE_DIR "${SCHROOT_LOCALE_DIR}"
// TODO: Remove when autotools are removed and sources updated.
#define PACKAGE_LOCALE_DIR SCHROOT_LOCALE_DIR
#cmakedefine SCHROOT_DATA_DIR "${SCHROOT_DATA_DIR}"
#cmakedefine SCHROOT_MODULE_DIR "${SCHROOT_MODULE_DIR}"

/* Translation catalogue name */
#define SBUILD_MESSAGE_CATALOGUE "schroot"

#endif /* SBUILD_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
