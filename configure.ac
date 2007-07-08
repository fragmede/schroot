#                                                              -*- Autoconf -*-
# Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>
#
# schroot is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# schroot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307  USA
#
# Process this file with autoconf to produce a configure script.
#
dnl MAKING RELEASES (a step-by-step guide!)
dnl ===============
dnl
dnl Since the last release:
dnl 1. if only source code (not the interface) has changed, set
dnl      SBUILD_MICRO_VERSION += 1;
dnl      SBUILD_INTERFACE_AGE += 1;
dnl 2. if any functions have been added, removed, or changed, set
dnl      SBUILD_INTERFACE_AGE = 0;
dnl      SBUILD_CURRENT_INTERFACE += 1;
dnl 3. if interfaces have been added, set
dnl      SBUILD_BINARY_AGE += 1;
dnl 4. if interfaces have been removed, set
dnl      SBUILD_BINARY_AGE = 0;
dnl
dnl For more detailed information, see the libtool info documentation.
dnl
pushdef([SBUILD_NAME],              [schroot])
pushdef([SBUILD_MAJOR_VERSION],     [1])
pushdef([SBUILD_MINOR_VERSION],     [1])
pushdef([SBUILD_MICRO_VERSION],     [5])
pushdef([SBUILD_EXTRA_VERSION],     [])
pushdef([SBUILD_CURRENT_INTERFACE], [1])
pushdef([SBUILD_INTERFACE_AGE],     [0])
pushdef([SBUILD_BINARY_AGE],        [0])
pushdef([SBUILD_VERSION], SBUILD_MAJOR_VERSION.SBUILD_MINOR_VERSION.SBUILD_MICRO_VERSION[]SBUILD_EXTRA_VERSION)
dnl
dnl The minimum version of autoconf required.
AC_PREREQ(2.59)
dnl Initialise autoconf with the package name, version and
dnl bug-reporting address.
AC_INIT(SBUILD_NAME, SBUILD_VERSION, [buildd-tools-devel@lists.alioth.debian.org])
dnl For safety, check we are in the right directory by
dnl checking for a known unique file.
AC_CONFIG_SRCDIR([sbuild/sbuild-session.cc])
dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])
dnl Put macro definitions here.
AC_CONFIG_HEADER([config.h])
AC_CONFIG_HEADER([sbuild/sbuild-config.h])

AC_COPYRIGHT(
[
Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>

Parts of this configure script come from the 'configure.ac' file, and
are not based on freely-redistributable m4 macros; you can redistribute
them and/or modify them under the terms of the GNU General Public
as published by the Free Software Foundation; either version 2, or (at
your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
])

[SBUILD_MAJOR_VERSION]=SBUILD_MAJOR_VERSION
[SBUILD_MINOR_VERSION]=SBUILD_MINOR_VERSION
[SBUILD_MICRO_VERSION]=SBUILD_MICRO_VERSION
[SBUILD_EXTRA_VERSION]=SBUILD_EXTRA_VERSION
[SBUILD_CURRENT_INTERFACE]=SBUILD_CURRENT_INTERFACE
[SBUILD_INTERFACE_AGE]=SBUILD_INTERFACE_AGE
[SBUILD_BINARY_AGE]=SBUILD_BINARY_AGE
[SBUILD_VERSION]=SBUILD_VERSION
popdef([SBUILD_MAJOR_VERSION])
popdef([SBUILD_MINOR_VERSION])
popdef([SBUILD_MICRO_VERSION])
popdef([SBUILD_EXTRA_VERSION])
popdef([SBUILD_CURRENT_INTERFACE])
popdef([SBUILD_INTERFACE_AGE])
popdef([SBUILD_BINARY_AGE])
popdef([SBUILD_NAME])
popdef([SBUILD_VERSION])

AC_SUBST([SBUILD_MAJOR_VERSION])
AC_SUBST([SBUILD_MINOR_VERSION])
AC_SUBST([SBUILD_MICRO_VERSION])
AC_SUBST([SBUILD_CURRENT_INTERFACE])
AC_SUBST([SBUILD_INTERFACE_AGE])
AC_SUBST([SBUILD_BINARY_AGE])
AC_SUBST([SBUILD_VERSION])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE([1.9 gnu check-news dist-bzip2 no-dist-gzip tar-pax])
AM_MAINTAINER_MODE
SCHROOT_RELEASE_DATE

AC_MSG_CHECKING([whether to enable dchroot compatibility])
enable_dchroot_compat="no"
AC_ARG_ENABLE([dchroot], [AS_HELP_STRING([--enable-dchroot], [Enable dchroot compatibility])],
              [ case "${enableval}" in
                yes) enable_dchroot_compat="yes" ;;
                no)  enable_dchroot_compat="no" ;;
                unset)  enable_dchroot_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-dchroot]) ;;
                esac])
AC_MSG_RESULT([$enable_dchroot_compat])
AM_CONDITIONAL([BUILD_DCHROOT], [test "$enable_dchroot_compat" = "yes"])

AC_MSG_CHECKING([whether to enable dchroot-dsa compatibility])
enable_dchroot_dsa_compat="no"
AC_ARG_ENABLE([dchroot-dsa], [AS_HELP_STRING([--enable-dchroot-dsa], [Enable dchroot-dsa compatibility])],
              [ case "${enableval}" in
                yes) enable_dchroot_dsa_compat="yes" ;;
                no)  enable_dchroot_dsa_compat="no" ;;
                unset)  enable_dchroot_dsa_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-dchroot-dsa]) ;;
                esac])
AC_MSG_RESULT([$enable_dchroot_dsa_compat])
AM_CONDITIONAL([BUILD_DCHROOT_DSA], [test "$enable_dchroot_dsa_compat" = "yes"])
AM_CONDITIONAL([BUILD_LIBDCHROOT], [test "$enable_dchroot_compat" = "yes" || test "$enable_dchroot_dsa_compat" = "yes"])


AC_MSG_CHECKING([whether to enable csbuild])
enable_csbuild_compat="no"
AC_ARG_ENABLE([csbuild], [AS_HELP_STRING([--enable-csbuild], [Enable csbuild])],
              [ case "${enableval}" in
                yes) enable_csbuild_compat="yes" ;;
                no)  enable_csbuild_compat="no" ;;
                unset)  enable_csbuild_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-csbuild]) ;;
                esac])
AC_MSG_RESULT([$enable_csbuild_compat])
AM_CONDITIONAL([BUILD_CSBUILD], [test "$enable_csbuild_compat" = "yes"])

AC_MSG_CHECKING([whether to enable debugging messages])
enable_debug="no"
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [Enable debugging messages])],
              [ case "${enableval}" in
                yes) enable_debug="yes" ;;
                no)  enable_debug="no" ;;
                unset)  enable_debug="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
                esac])
AC_MSG_RESULT([$enable_debug])
if test "$enable_debug" = "yes"; then
  AC_DEFINE_UNQUOTED([SBUILD_DEBUG], [1], [Enable debugging])
fi

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_GNU_GETTEXT_VERSION([0.14.5])
AM_GNU_GETTEXT([external])
AC_PATH_PROG([FIND], [find])
AC_PATH_PROG([XARGS], [xargs])
AC_PATH_PROG([DOXYGEN], [doxygen])
AM_CONDITIONAL([USE_DOXYGEN], [test -n "$DOXYGEN"])

# Checks for libraries.
AH_TEMPLATE(HAVE_UUID, [Is libuuid available])
PKG_CHECK_MODULES([UUID], [uuid],
                  [AC_DEFINE(HAVE_UUID)
		   HAVE_UUID=true],
                  [HAVE_UUID=false])

AM_PATH_CPPUNIT([1.10.0], [HAVE_CPPUNIT=true])
AM_CONDITIONAL([USE_UNIT_TESTS], [test -n "$HAVE_CPPUNIT"])

SCHROOT_CFLAGS="$UUID_CFLAGS"
AC_SUBST([SCHROOT_CFLAGS])

# Checks for header files.
AC_CHECK_HEADERS([tr1/memory])

AC_CHECK_HEADERS([boost/shared_ptr.hpp],, [
  if test $ac_cv_header_tr1_memory = yes; then
    :
  else
    AC_MSG_ERROR([Boost.shared_ptr (Boost C++ Libraries) is not installed, but is required by schroot])
  fi])

AC_CHECK_HEADERS([tr1/tuple])

AC_CHECK_HEADERS([boost/tuple/tuple.hpp],, [
  if test $ac_cv_header_tr1_memory = yes; then
    :
  else
    AC_MSG_ERROR([Boost.Tuple (Boost C++ Libraries) is not installed, but is required by schroot])
  fi])

AC_CHECK_HEADERS([boost/format.hpp],, [AC_MSG_ERROR([Boost.Format (Boost C++ Libraries) is not installed, but is required by schroot])])
AC_CHECK_HEADERS([boost/program_options.hpp],, [AC_MSG_ERROR([Boost.Program_options (Boost C++ Libraries) is not installed, but is required by schroot])])
AC_CHECK_HEADERS([boost/type_traits.hpp],, [AC_MSG_ERROR([Boost.TypeTraits (Boost C++ Libraries) is not installed, but is required by schroot])])

AC_CHECK_HEADERS([ext/stdio_filebuf.h],, [AC_MSG_ERROR([__gnu_cxx::stdio_filebuf (GNU libstdc++) is not installed, but is required by schroot])])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_LIB([pam], [pam_authenticate],
             [PAM_LIBS="-lpam"],
	     [AC_MSG_FAILURE([libpam (Linux-PAM) is not installed, but is required by schroot])])
AC_SUBST([PAM_LIBS])

AC_CHECK_LIB([lockdev], [dev_lock],
             [LOCKDEV_LIBS="-llockdev"],
             [AC_MSG_FAILURE([liblockdev (lockdev) is not installed, but is required by schroot])])
AC_SUBST([LOCKDEV_LIBS])

AC_MSG_CHECKING([for boost::program_options::variables_map in -lboost_program_options-st])
saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lboost_program_options-st"
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                               [boost::program_options::variables_map::variables_map dummy()])],
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_program_options-st"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_program_options (Boost C++ Libraries) is not installed, but is required by schroot])])
LDFLAGS="${saved_ldflags}"

AC_MSG_CHECKING([for boost::program_options::options_description::options() in -lboost_program_options-st])
saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lboost_program_options-st"
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                               [boost::program_options::options_description testgrp("test group");
                                bool notused = testgrp.options().empty();
])],
               [AC_MSG_RESULT([yes])
                BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS="current"],
               [AC_MSG_RESULT([no])
                BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS="old"])
LDFLAGS="${saved_ldflags}"
AH_TEMPLATE(BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD, [Set if boost::program_options::options_description::options() is not available])
if test "$BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS" = "old"; then
  AC_DEFINE(BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD, 1)
fi

AC_MSG_CHECKING([for boost::regex in -lboost_regex-st])
saved_ldflags="${LDFLAGS}"
LDFLAGS="${LDFLAGS} -lboost_regex-st"
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <boost/regex.hpp>],
                               [boost::regex("^foo[bar]$")])],
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_regex-st"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_regex (Boost C++ Libraries) is not installed, but is required by schroot])])
LDFLAGS="${saved_ldflags}"

AC_SUBST([BOOST_LIBS])

AC_MSG_CHECKING([__gnu_cxx::stdio_filebuf construction semantics])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <ext/stdio_filebuf.h>
#include <unistd.h>],
                                [__gnu_cxx::stdio_filebuf<char> fdbuf(STDOUT_FILENO, std::ios::out)])],
               [AC_MSG_RESULT([new (GCC >= 3.4)])
	        STDIO_FILEBUF="new"],
               [AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <ext/stdio_filebuf.h>
#include <unistd.h>],
		               [__gnu_cxx::stdio_filebuf<char> fdbuf(STDOUT_FILENO, std::ios::out, true, BUFSIZ)])],
                               [AC_MSG_RESULT([old (GCC < 3.4)])
	                        STDIO_FILEBUF="old"],
                               [AC_MSG_FAILURE([__gnu_cxx::stdio_filebuf (GNU libstdc++) is not installed, but is required by schroot])])])

AH_TEMPLATE(SCHROOT_FILEBUF_OLD, [Set if old __gnu_cxx::stdio_filebuf construction semantics are required])
if test "$STDIO_FILEBUF" = "old"; then
  AC_DEFINE(SCHROOT_FILEBUF_OLD, 1)
fi

dnl Set PACKAGE_LOCALE_DIR in config.h
AH_TEMPLATE(PACKAGE_LOCALE_DIR, [Package locale directory])
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ["${ac_default_prefix}/share/locale"])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ["${prefix}/share/locale"])
fi

dnl Set PACKAGE_DATA_DIR in config.h.
AH_TEMPLATE(PACKAGE_DATA_DIR, [Package data directory])
if test "x${datadir}" = 'x${prefix}/share' || test "x${datadir}" = 'x${datarootdir}'; then
  if test "x${prefix}" = "xNONE"; then
    PACKAGE_DATA_DIR="${ac_default_prefix}/share/${PACKAGE}"
  else
    PACKAGE_DATA_DIR="${prefix}/share/${PACKAGE}"
  fi
else
  PACKAGE_DATA_DIR="${datadir}/${PACKAGE}"
fi

AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${PACKAGE_DATA_DIR}")

dnl Set PACKAGE_LIB_DIR in config.h.
AH_TEMPLATE(PACKAGE_LIB_DIR, [Package lib directory])
if test "x${libdir}" = 'x${exec_prefix}/lib'; then
  if test "x${exec_prefix}" = "xNONE"; then
    if test "x${prefix}" = "xNONE"; then
      PACKAGE_LIB_DIR="${ac_default_prefix}/lib/${PACKAGE}"
    else
      PACKAGE_LIB_DIR="${prefix}/lib/${PACKAGE}"
    fi
  else
    PACKAGE_LIB_DIR="${exec_prefix}/lib/${PACKAGE}"
  fi
else
  PACKAGE_LIB_DIR="${libdir}/${PACKAGE}"
fi

dnl Set PACKAGE_LIBEXEC_DIR in config.h.
AH_TEMPLATE(PACKAGE_LIBEXEC_DIR, [Package libexec directory])
if test "x${libexecdir}" = 'x${exec_prefix}/libexec'; then
  if test "x${exec_prefix}" = "xNONE"; then
    if test "x${prefix}" = "xNONE"; then
      PACKAGE_LIBEXEC_DIR="${ac_default_prefix}/libexec/${PACKAGE}"
    else
      PACKAGE_LIBEXEC_DIR="${prefix}/libexec/${PACKAGE}"
    fi
  else
    PACKAGE_LIBEXEC_DIR="${exec_prefix}/libexec/${PACKAGE}"
  fi
else
  PACKAGE_LIBEXEC_DIR="${libexecdir}/${PACKAGE}"
fi

SCHROOT_LIBEXEC_DIR="${PACKAGE_LIBEXEC_DIR}"
AC_SUBST(SCHROOT_LIBEXEC_DIR)
AH_TEMPLATE(SCHROOT_LIBEXEC_DIR, [Package libexec directory])
AC_DEFINE_UNQUOTED(SCHROOT_LIBEXEC_DIR, ["$SCHROOT_LIBEXEC_DIR"])

dnl Set PACKAGE_LOCALSTATE_DIR in config.h.
AH_TEMPLATE(PACKAGE_LOCALSTATE_DIR, [Package localstate directory])
if test "x${localstatedir}" = 'x${prefix}/var'; then
  if test "x${prefix}" = "xNONE"; then
    if test "x${prefix}" = "xNONE"; then
      PACKAGE_LOCALSTATE_DIR="${ac_default_prefix}/var"
    else
      PACKAGE_LOCALSTATE_DIR="${prefix}/var"
    fi
  else
    PACKAGE_LOCALSTATE_DIR="${prefix}/var"
  fi
else
  PACKAGE_LOCALSTATE_DIR="${localstatedir}"
fi

SCHROOT_MOUNT_DIR="${PACKAGE_LOCALSTATE_DIR}/lib/${PACKAGE}/mount"
AC_SUBST([SCHROOT_MOUNT_DIR])
AH_TEMPLATE(SCHROOT_MOUNT_DIR, [schroot mount directory])
AC_DEFINE_UNQUOTED(SCHROOT_MOUNT_DIR, ["$SCHROOT_MOUNT_DIR"])

SCHROOT_SESSION_DIR="${PACKAGE_LOCALSTATE_DIR}/lib/${PACKAGE}/session"
AC_SUBST([SCHROOT_SESSION_DIR])
AH_TEMPLATE(SCHROOT_SESSION_DIR, [schroot session directory])
AC_DEFINE_UNQUOTED(SCHROOT_SESSION_DIR, ["$SCHROOT_SESSION_DIR"])

dnl Set PACKAGE_SYSCONF_DIR in config.h.
AH_TEMPLATE(PACKAGE_SYSCONF_DIR, [Package system configuration directory])
if test "x${sysconfdir}" = 'x${prefix}/etc'; then
  if test "x${prefix}" = "xNONE"; then
    PACKAGE_SYSCONF_DIR="${ac_default_prefix}/etc"
  else
    PACKAGE_SYSCONF_DIR="${prefix}/etc"
  fi
else
  PACKAGE_SYSCONF_DIR="${sysconfdir}"
fi
PACKAGE_SYSCONF_DIR="${PACKAGE_SYSCONF_DIR}/schroot"
AC_DEFINE_UNQUOTED([PACKAGE_SYSCONF_DIR], ["$PACKAGE_SYSCONF_DIR"])
AC_SUBST([PACKAGE_SYSCONF_DIR])

SCHROOT_CONF="${PACKAGE_SYSCONF_DIR}/schroot.conf"
AC_SUBST([SCHROOT_CONF])

AH_TEMPLATE(SCHROOT_CONF, [schroot config file path])
AC_DEFINE_UNQUOTED(SCHROOT_CONF, ["$SCHROOT_CONF"])

SCHROOT_CONF_SETUP_D="${PACKAGE_SYSCONF_DIR}/setup.d"
AC_SUBST([SCHROOT_CONF_SETUP_D])

AH_TEMPLATE(SCHROOT_CONF_SETUP_D, [schroot chroot setup directory])
AC_DEFINE_UNQUOTED(SCHROOT_CONF_SETUP_D, ["$SCHROOT_CONF_SETUP_D"])

SCHROOT_CONF_EXEC_D="${PACKAGE_SYSCONF_DIR}/exec.d"
AC_SUBST([SCHROOT_CONF_EXEC_D])

AH_TEMPLATE(SCHROOT_CONF_EXEC_D, [schroot chroot exec directory])
AC_DEFINE_UNQUOTED(SCHROOT_CONF_EXEC_D, ["$SCHROOT_CONF_EXEC_D"])

AH_TEMPLATE(SCHROOT_DATADIR, [Package data directory])
SCHROOT_DATADIR="${PACKAGE_DATA_DIR}/${PACKAGE_VERSION}"
AC_DEFINE_UNQUOTED(SCHROOT_DATADIR, ["$SCHROOT_DATADIR"])

AH_TEMPLATE(SCHROOT_MODULEDIR, [Package module directory])
SCHROOT_MODULEDIR="${PACKAGE_LIB_DIR}/${PACKAGE_VERSION}/modules"
AC_DEFINE_UNQUOTED(SCHROOT_MODULEDIR, ["$SCHROOT_MODULEDIR"])

if test "x${sysconfdir}" = 'x${prefix}/etc'; then
  if test "x${prefix}" = "xNONE"; then
    DCHROOT_SYSCONF_DIR="${ac_default_prefix}/etc"
  else
    DCHROOT_SYSCONF_DIR="${prefix}/etc"
  fi
else
  DCHROOT_SYSCONF_DIR="${sysconfdir}"
fi

DCHROOT_CONF="${DCHROOT_SYSCONF_DIR}/dchroot.conf"
AC_SUBST([DCHROOT_CONF])

AH_TEMPLATE(DCHROOT_CONF, [dchroot config file path])
AC_DEFINE_UNQUOTED(DCHROOT_CONF, ["$DCHROOT_CONF"])

CSBUILD_CONF="${CSBUILD_SYSCONF_DIR}/csbuild.conf"
AC_SUBST([CSBUILD_CONF])

AH_TEMPLATE(CSBUILD_CONF, [csbuild config file path])
AC_DEFINE_UNQUOTED(CSBUILD_CONF, ["$CSBUILD_CONF"])

dnl Configure which files to generate.
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/sbuild.dox])
AC_CONFIG_FILES([doc/schroot.dox])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([sbuild/Makefile])
AC_CONFIG_FILES([sbuild/sbuild.pc])
AC_CONFIG_FILES([bin/Makefile])
AC_CONFIG_FILES([bin/schroot-base/Makefile])
AC_CONFIG_FILES([bin/schroot/Makefile])
AC_CONFIG_FILES([bin/schroot/pam/Makefile])
AC_CONFIG_FILES([bin/schroot/exec/Makefile])
AC_CONFIG_FILES([bin/schroot/setup/Makefile])
AC_CONFIG_FILES([bin/schroot/schroot.1])
AC_CONFIG_FILES([bin/schroot/schroot-setup.5])
AC_CONFIG_FILES([bin/schroot/schroot.conf.5])
AC_CONFIG_FILES([bin/schroot-listmounts/Makefile])
AC_CONFIG_FILES([bin/schroot-releaselock/Makefile])
AC_CONFIG_FILES([bin/dchroot/Makefile])
AC_CONFIG_FILES([bin/dchroot/dchroot.1])
AC_CONFIG_FILES([bin/dchroot-dsa/Makefile])
AC_CONFIG_FILES([bin/dchroot-dsa/dchroot-dsa.1])
AC_CONFIG_FILES([bin/csbuild/Makefile])
AC_CONFIG_FILES([bin/csbuild/csbuild.1])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([Makefile])
dnl Output the generated config.status script.
AC_OUTPUT
