#                                                              -*- Autoconf -*-
# Copyright © 2004-2009  Roger Leigh <rleigh@debian.org>
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
#
# Process this file with autoconf to produce a configure script.
#
#####################################################################
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
dnl m4 magic from Eric Blake <ebb9@byu.net>, prior to automake inclusion
m4_define([sbuild_m4_chomp],
[m4_format([[%.*s]], m4_bregexp(m4_translit([$1], [
]]m4_dquote(_m4_defn([m4_cr_all]))[, [/]]m4_format([%255s], [])[),
  [/*$]), [$1])])
m4_define([sbuild_m4_esyscmd_s],
[sbuild_m4_chomp(m4_esyscmd([$1]))])
AC_PREREQ(2.59)
dnl Quoting the first argument results in a bizarrely corrupted package tarname
AC_INIT(sbuild_m4_esyscmd_s([sed -ne '/^Package:/{s/Package:[[:space:]][[:space:]]*//p;q}' VERSION]),
        [sbuild_m4_esyscmd_s([sed -ne '/^Version:/{s/Version:[[:space:]][[:space:]]*//p;q}' VERSION])],
        [buildd-tools-devel@lists.alioth.debian.org])
pushdef([SBUILD_CURRENT_INTERFACE], [1])
pushdef([SBUILD_INTERFACE_AGE],     [0])
pushdef([SBUILD_BINARY_AGE],        [0])
dnl For safety, check we are in the right directory by
dnl checking for a known unique file.
AC_CONFIG_SRCDIR([sbuild/sbuild-session.cc])
dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])
dnl Add config headers.
AC_CONFIG_HEADER([config.h])
AC_CONFIG_HEADER([sbuild/sbuild-config.h])

AC_COPYRIGHT(
[
Copyright © 2004-2009  Roger Leigh <rleigh@debian.org>

Parts of this configure script come from the 'configure.ac' file, and
are not based on freely-redistributable m4 macros; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

This software is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
])

[SBUILD_CURRENT_INTERFACE]=SBUILD_CURRENT_INTERFACE
[SBUILD_INTERFACE_AGE]=SBUILD_INTERFACE_AGE
[SBUILD_BINARY_AGE]=SBUILD_BINARY_AGE
popdef([SBUILD_CURRENT_INTERFACE])
popdef([SBUILD_INTERFACE_AGE])
popdef([SBUILD_BINARY_AGE])

AC_SUBST([SBUILD_CURRENT_INTERFACE])
AC_SUBST([SBUILD_INTERFACE_AGE])
AC_SUBST([SBUILD_BINARY_AGE])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE([1.10 gnu check-news dist-bzip2 no-dist-gzip tar-pax])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
RELEASE_DATE='sbuild_m4_esyscmd_s(date --date='sbuild_m4_esyscmd_s([sed -ne '/^Release-Date:/{s/Release-Date:[[:space:]][[:space:]]*//p;q}' VERSION])' '+%s')'
RELEASE_DATE_S='sbuild_m4_esyscmd_s([sed -ne '/^Release-Date:/{s/Release-Date:[[:space:]][[:space:]]*//p;q}' VERSION])'
AC_DEFINE_UNQUOTED([RELEASE_DATE_S], ["$RELEASE_DATE_S"], [Package release date (string).])
AC_DEFINE_UNQUOTED([RELEASE_DATE], [$RELEASE_DATE], [Package release date (integer).])
AC_SUBST([RELEASE_DATE])
AC_SUBST([RELEASE_DATE_S])

AC_MSG_CHECKING([whether to enable dchroot compatibility])
AC_ARG_ENABLE([dchroot], [AS_HELP_STRING([--enable-dchroot], [Enable dchroot compatibility])],
              [ case "${enableval}" in
                yes) enable_dchroot_compat="yes" ;;
                no)  enable_dchroot_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-dchroot]) ;;
                esac ],
              [ enable_dchroot_compat="no" ])
AC_MSG_RESULT([$enable_dchroot_compat])
AM_CONDITIONAL([BUILD_DCHROOT], [test "$enable_dchroot_compat" = "yes"])

AC_MSG_CHECKING([whether to enable dchroot-dsa compatibility])
AC_ARG_ENABLE([dchroot-dsa], [AS_HELP_STRING([--enable-dchroot-dsa], [Enable dchroot-dsa compatibility])],
              [ case "${enableval}" in
                yes) enable_dchroot_dsa_compat="yes" ;;
                no)  enable_dchroot_dsa_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-dchroot-dsa]) ;;
                esac ],
	      [ enable_dchroot_dsa_compat="no" ])
AC_MSG_RESULT([$enable_dchroot_dsa_compat])
AM_CONDITIONAL([BUILD_DCHROOT_DSA], [test "$enable_dchroot_dsa_compat" = "yes"])
AM_CONDITIONAL([BUILD_LIBDCHROOT], [test "$enable_dchroot_compat" = "yes" || test "$enable_dchroot_dsa_compat" = "yes"])

AC_MSG_CHECKING([whether to enable csbuild])
AC_ARG_ENABLE([csbuild], [AS_HELP_STRING([--enable-csbuild], [Enable csbuild])],
              [ case "${enableval}" in
                yes) enable_csbuild_compat="yes" ;;
                no)  enable_csbuild_compat="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-csbuild]) ;;
                esac ],
              [ enable_csbuild_compat="no" ])

AC_MSG_RESULT([$enable_csbuild_compat])
AM_CONDITIONAL([BUILD_CSBUILD], [test "$enable_csbuild_compat" = "yes"])

AC_MSG_CHECKING([whether to enable debugging messages])
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [Enable debugging messages])],
              [ case "${enableval}" in
                yes) enable_debug="yes" ;;
                no)  enable_debug="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
                esac ],
              [ enable_debug="no" ])
AC_MSG_RESULT([$enable_debug])
if test "$enable_debug" = "yes"; then
  AC_DEFINE_UNQUOTED([SBUILD_DEBUG], [1], [Enable debugging])
fi

default_environment_filter='^(BASH_ENV|CDPATH|ENV|HOSTALIASES|IFS|KRB5_CONFIG|KRBCONFDIR|KRBTKFILE|KRB_CONF|LD_.*|LOCALDOMAIN|NLSPATH|PATH_LOCALE|RES_OPTIONS|TERMINFO|TERMINFO_DIRS|TERMPATH)$'

AC_MSG_CHECKING([whether to enable environment filtering regex])
AC_ARG_ENABLE([environment-filter], [AS_HELP_STRING([--enable-environment-filter], [Enable default environment filtering (regex)])],
              [ case "${enableval}" in
                yes) enable_environment_filter="$default_environment_filter" ;;
                no)  enable_environment_filter="" ;;
                *) enable_environment_filter="${enableval}" ;;
                esac ],
              [ enable_environment_filter="$default_environment_filter" ])
AC_MSG_RESULT([$enable_environment_filter])
AH_TEMPLATE(SBUILD_DEFAULT_ENVIRONMENT_FILTER, [Default regular expression used to filter user environment])
AC_DEFINE_UNQUOTED([SBUILD_DEFAULT_ENVIRONMENT_FILTER], ["$enable_environment_filter"])

AC_MSG_CHECKING([whether to enable support for PAM authentication])
AC_ARG_ENABLE([pam], [AS_HELP_STRING([--enable-pam], [Enable support for PAM authentication (requires libpam)])],
              [ case "${enableval}" in
                yes) enable_pam="yes" ;;
                no)  enable_pam="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-pam]) ;;
                esac],
              [enable_pam="auto"])
AC_MSG_RESULT([$enable_pam])

AC_MSG_CHECKING([whether to enable support for block devices])
AC_ARG_ENABLE([block-device], [AS_HELP_STRING([--enable-block-device], [Enable support for block devices (requires liblockdev)])],
              [ case "${enableval}" in
                yes) enable_blockdev="yes" ;;
                no)  enable_blockdev="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-blockdev]) ;;
                esac],
              [enable_blockdev="auto"])
AC_MSG_RESULT([$enable_blockdev])

AC_MSG_CHECKING([whether to enable support for LVM snapshots])
AC_ARG_ENABLE([lvm-snapshot], [AS_HELP_STRING([--enable-lvm-snapshot], [Enable support for LVM snapshots (requires LVM and liblockdev)])],
              [ case "${enableval}" in
                yes) enable_lvmsnapshot="yes"
		     enable_blockdev="yes" ;;
                no)  enable_lvmsnapshot="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-lvm-snapshot]) ;;
                esac],
              [enable_lvmsnapshot="auto"])
AC_MSG_RESULT([$enable_lvmsnapshot])

AC_MSG_CHECKING([whether to enable support for BTRFS snapshots])
AC_ARG_ENABLE([btrfs-snapshot], [AS_HELP_STRING([--enable-btrfs-snapshot], [Enable support for btrfs snapshots (requires btrfs)])],
              [ case "${enableval}" in
                yes) enable_btrfssnapshot="yes"
		     enable_blockdev="yes" ;;
                no)  enable_btrfssnapshot="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-btrfs-snapshot]) ;;
                esac],
              [enable_btrfssnapshot="auto"])
AC_MSG_RESULT([$enable_btrfssnapshot])

AC_MSG_CHECKING([whether to enable support for loopback mounts])
AC_ARG_ENABLE([loopback], [AS_HELP_STRING([--enable-loopback], [Enable support for loopback mounts])],
              [ case "${enableval}" in
                yes) enable_loopback="yes" ;;
                no)  enable_loopback="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-loopback]) ;;
                esac],
              [enable_loopback="auto"])
AC_MSG_RESULT([$enable_loopback])

AC_MSG_CHECKING([whether to enable support for UUIDs in session names])
AC_ARG_ENABLE([uuid], [AS_HELP_STRING([--enable-uuid], [Enable support for UUIDs])],
              [ case "${enableval}" in
                yes) enable_uuid="yes" ;;
                no)  enable_uuid="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-uuid]) ;;
                esac],
              [enable_uuid="auto"])
AC_MSG_RESULT([$enable_uuid])

AC_MSG_CHECKING([whether to enable support for union mounts])
AC_ARG_ENABLE([union], [AS_HELP_STRING([--enable-union], [Enable support for union mounts])],
              [ case "${enableval}" in
                yes) enable_union="yes" ;;
                no)  enable_union="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-union]) ;;
                esac],
              [enable_union="auto"])
AC_MSG_RESULT([$enable_union])

AC_MSG_CHECKING([whether to enable doxygen documentation])
AC_ARG_ENABLE([doxygen], [AS_HELP_STRING([--enable-doxygen], [Enable doxygen documentation])],
              [ case "${enableval}" in
                yes) enable_doxygen="yes" ;;
                no)  enable_doxygen="no" ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-doxygen]) ;;
                esac],
	      [ enable_doxygen="no" ])
AC_MSG_RESULT([$enable_doxygen])
AM_CONDITIONAL([BUILD_DOXYGEN], [test "$enable_doxygen" = "yes"])

AC_MSG_CHECKING([for bash completion directory])
bashcompletiondir='${sysconfdir}/bash_completion.d'
AC_ARG_WITH([bash-completion-dir], [AS_HELP_STRING([--with-bash-completion-dir], [bash shell completion directory])],
              [bashcompletiondir="${withval}"])
AC_MSG_RESULT([$bashcompletiondir])
AC_SUBST([bashcompletiondir])

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
ACX_PTHREAD([], [AC_MSG_ERROR([POSIX thread support is required for correct std::tr1::shared_ptr operation])])
AC_CANONICAL_HOST
AC_DISABLE_SHARED
AC_ENABLE_STATIC
AC_PROG_LIBTOOL
AM_GNU_GETTEXT_VERSION([0.16])
AM_GNU_GETTEXT([external])
AC_PATH_PROG([FIND], [find])
AC_PATH_PROG([XARGS], [xargs])
AC_PATH_PROG([DOXYGEN], [doxygen])
HAVE_LVM="yes"
AC_PATH_PROG([LVCREATE], [lvcreate], [HAVE_LVM="no"], [$PATH:/sbin:/usr/sbin])
AC_PATH_PROG([LVREMOVE], [lvremove], [HAVE_LVM="no"], [$PATH:/sbin:/usr/sbin])
HAVE_BTRFS="yes"
AC_PATH_PROG([BTRFS], [btrfs], [HAVE_BTRFS="no"], [$PATH:/sbin:/usr/sbin])
AC_PATH_PROG([BTRFSCTL], [btrfsctl], [HAVE_BTRFS="no"], [$PATH:/sbin:/usr/sbin])
HAVE_LOOPBACK="yes"
AC_PATH_PROG([LOSETUP], [losetup], [HAVE_LOOPBACK="no"], [$PATH:/sbin:/usr/sbin])

if test "$enable_doxygen" = "yes" && test -z "$DOXYGEN"; then
  AC_MSG_FAILURE([doxygen is not installed, but is required by schroot])
fi;
AM_CONDITIONAL([BUILD_DOXYGEN], [test -n "$DOXYGEN" && test "$enable_doxygen" = "yes"])


# Check for host platform
AC_MSG_CHECKING([for supported host platform type])
PLATFORM="generic"
case $host_os in
    linux*):
	PLATFORM="linux";;
    freebsd* | k*bsd*-gnu) :
	PLATFORM="freebsd";;
esac
AC_MSG_RESULT([$PLATFORM])
AC_SUBST([PLATFORM], [$PLATFORM])

AM_CONDITIONAL([PLATFORM_GENERIC], [test "$PLATFORM" = "generic"])
AM_CONDITIONAL([PLATFORM_LINUX], [test "$PLATFORM" = "linux"])
AM_CONDITIONAL([PLATFORM_FREEBSD], [test "$PLATFORM" = "freebsd"])

AH_TEMPLATE(SBUILD_PLATFORM, [Platform type, used to modify run-time platform-specific behaviour])
AC_DEFINE_UNQUOTED(SBUILD_PLATFORM, ["$PLATFORM"])

AH_TEMPLATE(SBUILD_HOST, [Host GNU architecture triplet])
AH_TEMPLATE(SBUILD_HOST_OS, [Host OS])
AH_TEMPLATE(SBUILD_HOST_VENDOR, [Host vendor])
AH_TEMPLATE(SBUILD_HOST_CPU, [Host CPU])
AC_DEFINE_UNQUOTED(SBUILD_HOST, ["$host"])
AC_DEFINE_UNQUOTED(SBUILD_HOST_OS, ["$host_os"])
AC_DEFINE_UNQUOTED(SBUILD_HOST_VENDOR, ["$host_vendor"])
AC_DEFINE_UNQUOTED(SBUILD_HOST_CPU, ["$host_cpu"])


# Checks for libraries.
PKG_CHECK_MODULES([UUID], [uuid],
                  [AC_DEFINE(HAVE_UUID)
		   HAVE_UUID=yes],
                  [HAVE_UUID=no])

AM_PATH_CPPUNIT([1.10.0], [HAVE_CPPUNIT=yes])
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

AC_CHECK_HEADERS([sys/personality.h],
	         [PERSONALITY_HEADER="yes"],
	         [PERSONALITY_HEADER="no"])

AC_CHECK_HEADERS([security/pam_appl.h],
	         [PAM_HEADER="yes"],
	         [PAM_HEADER="no"])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_LIB([pam], [pam_authenticate],
             [PAM_LIBS="-lpam"
	      PAM_FUNC="yes"],
	     [PAM_LIBS=""
	      PAM_FUNC="no"])
AC_SUBST([PAM_LIBS])

AC_CHECK_LIB([lockdev], [dev_lock],
             [LOCKDEV_LIBS="-llockdev"
	      HAVE_LOCKDEV="yes"],
             [LOCKDEV_LIBS=""
	      HAVE_LOCKDEV="no"])
AC_SUBST([LOCKDEV_LIBS])
AH_TEMPLATE(SBUILD_FEATURE_DEVLOCK, [Set if the device locking with liblockdev is available])
if test "$HAVE_LOCKDEV" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_DEVLOCK, 1)
fi

AC_CHECK_FUNC([personality],
              [PERSONALITY_FUNC="yes"],
	      [PERSONALITY_FUNC="no"])

# Determine which features to enable.
AC_MSG_CHECKING([whether to build personality support])
BUILD_PERSONALITY="yes"
if test "$PERSONALITY_HEADER" = "no"; then
  BUILD_PERSONALITY="no"
fi
if test "$PERSONALITY_FUNC" = "no"; then
  BUILD_PERSONALITY="no"
fi
AC_MSG_RESULT([$BUILD_PERSONALITY])

AM_CONDITIONAL([BUILD_PERSONALITY], [test "$BUILD_PERSONALITY" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_PERSONALITY, [Set if personality support is present])
if test "$BUILD_PERSONALITY" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_PERSONALITY, 1)
fi

AC_MSG_CHECKING([whether to build UUID support])
BUILD_UUID="yes"
if test "$enable_uuid" = "yes"; then
  BUILD_UUID="no"
fi
if test "$HAVE_UUID" = "no"; then
  BUILD_UUID="no"
fi
AC_MSG_RESULT([$BUILD_UUID])

AH_TEMPLATE(HAVE_UUID, [Set if UUID support is available])
if test "$BUILD_UUID" = "yes"; then
  AC_DEFINE(HAVE_UUID, 1)
fi

AC_MSG_CHECKING([whether to build PAM support])
BUILD_PAM="yes"
if test "$enable_pam" = "yes"; then
  if test "$PAM_HEADER" = "no"; then
    BUILD_PAM="no"
    AC_MSG_FAILURE([libpam (Linux-PAM) is not installed, but is required by schroot])
  fi
  if test "$PAM_FUNC" = "no"; then
    BUILD_PAM="no"
    AC_MSG_FAILURE([libpam (Linux-PAM) is not installed, but is required by schroot])
  fi
elif test "$enable_pam" = "no"; then
    BUILD_PAM="no"
elif test "$enable_pam" = "auto"; then
  if test "$PAM_HEADER" = "no"; then
    BUILD_PAM="no"
  fi
  if test "$PAM_FUNC" = "no"; then
    BUILD_PAM="no"
  fi
fi
AC_MSG_RESULT([$BUILD_PAM])

AM_CONDITIONAL([BUILD_PAM], [test "$BUILD_PAM" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_PAM, [Set if PAM support is available])
if test "$BUILD_PAM" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_PAM, 1)
fi

AC_MSG_CHECKING([whether to build block-device support])
BUILD_BLOCKDEV="yes"
if test "$enable_blockdev" = "yes"; then
  if test "$HAVE_LOCKDEV" = "no"; then
    BUILD_BLOCKDEV="no"
    AC_MSG_FAILURE([liblockdev (lockdev) is not installed, but is required by schroot])
  fi
elif test "$enable_blockdev" = "no"; then
    BUILD_BLOCKDEV="no"
elif test "$enable_blockdev" = "auto"; then
  if test "$HAVE_LOCKDEV" = "no"; then
    BUILD_BLOCKDEV="no"
  fi
fi
AC_MSG_RESULT([$BUILD_BLOCKDEV])

AM_CONDITIONAL([BUILD_BLOCKDEV], [test "$BUILD_BLOCKDEV" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_BLOCKDEV, [Set if the block-device chroot type is present])
if test "$BUILD_BLOCKDEV" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_BLOCKDEV, 1)
fi

AC_MSG_CHECKING([whether to build lvm-snapshot support])
BUILD_LVMSNAP="yes"
if test "$enable_lvmsnapshot" = "yes"; then
  if test "$HAVE_LVM" = "no"; then
    BUILD_LVMSNAP="no"
    AC_MSG_FAILURE([LVM (lvm) is not installed, but is required by schroot])
  fi
elif test "$enable_lvmsnapshot" = "no"; then
    BUILD_LVMSNAP="no"
elif test "$enable_lvmsnapshot" = "auto"; then
  if test "$HAVE_LOCKDEV" = "no"; then
    BUILD_LVMSNAP="no"
  fi
  if test "$HAVE_LVM" = "no"; then
    BUILD_LVMSNAP="no"
  fi
fi
AC_MSG_RESULT([$BUILD_LVMSNAP])

AM_CONDITIONAL([BUILD_LVMSNAP], [test "$BUILD_LVMSNAP" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_LVMSNAP, [Set if the lvm-snapshot chroot type is present])
if test "$BUILD_LVMSNAP" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_LVMSNAP, 1)
fi

AC_MSG_CHECKING([whether to build btrfs-snapshot support])
BUILD_BTRFSSNAP="yes"
if test "$enable_btrfssnapshot" = "yes"; then
  if test "$HAVE_BTRFS" = "no"; then
    BUILD_BTRFSSNAP="no"
    AC_MSG_FAILURE([BTRFS (btrfs) is not installed, but is required by schroot])
  fi
elif test "$enable_btrfssnapshot" = "no"; then
    BUILD_BTRFSSNAP="no"
elif test "$enable_btrfssnapshot" = "auto"; then
  if test "$HAVE_BTRFS" = "no"; then
    BUILD_BTRFSSNAP="no"
  fi
fi
AC_MSG_RESULT([$BUILD_BTRFSSNAP])

AM_CONDITIONAL([BUILD_BTRFSSNAP], [test "$BUILD_BTRFSSNAP" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_BTRFSSNAP, [Set if the btrfs-snapshot chroot type is present])
if test "$BUILD_BTRFSSNAP" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_BTRFSSNAP, 1)
fi

AC_MSG_CHECKING([whether to build loopback support])
BUILD_LOOPBACK="yes"
if test "$enable_loopbackshot" = "yes"; then
  if test "$HAVE_LOOPBACK" = "no"; then
    BUILD_LOOPBACK="no"
    AC_MSG_FAILURE([Loopback support is not avilable, but is required by schroot])
  fi
elif test "$enable_loopbackshot" = "no"; then
    BUILD_LOOPBACK="no"
elif test "$enable_loopbackshot" = "auto"; then
  if test "$HAVE_LOOPBACK" = "no"; then
    BUILD_LOOPBACK="no"
  fi
fi
AC_MSG_RESULT([$BUILD_LOOPBACK])

AM_CONDITIONAL([BUILD_LOOPBACK], [test "$BUILD_LOOPBACK" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_LOOPBACK, [Set if the loopback chroot type is present])
if test "$BUILD_LOOPBACK" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_LOOPBACK, 1)
fi

AC_MSG_CHECKING([whether to build union support])
BUILD_UNION="yes"
if test "$enable_union" = "yes"; then
  :
elif test "$enable_union" = "no"; then
  BUILD_UNION="no"
elif test "$enable_union" = "auto"; then
  BUILD_UNION="yes"
fi
AC_MSG_RESULT([$BUILD_UNION])

AM_CONDITIONAL([BUILD_UNION], [test "$BUILD_UNION" = "yes"])
AH_TEMPLATE(SBUILD_FEATURE_UNION, [Set if the union filesystem type is present])
if test "$BUILD_UNION" = "yes"; then
  AC_DEFINE(SBUILD_FEATURE_UNION, 1)
fi

AC_MSG_CHECKING([for boost::program_options::variables_map in -lboost_program_options])
saved_LIBS="${LIBS}"
LIBS="${saved_LIBS} -lboost_program_options"
define([testprog], [AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                                   [boost::program_options::variables_map::variables_map dummy()])])
AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_program_options"],
LIBS="${saved_LIBS} -lboost_program_options-mt"
[AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_program_options-mt"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_program_options (Boost C++ Libraries) is not installed, but is required by schroot])])])
LIBS="${saved_LIBS}"

AC_MSG_CHECKING([for boost::program_options::options_description::options() in -lboost_program_options])
saved_LIBS="${LIBS}"
LIBS="${saved_LIBS} -lboost_program_options"
define([testprog], [AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                                   [boost::program_options::options_description testgrp("test group");
                                    bool notused = testgrp.options().empty();
])])
AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
                BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS="current"],
LIBS="${saved_LIBS} -lboost_program_options"
[AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
                BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS="current"],
               [AC_MSG_RESULT([no])
                BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS="old"])])
LIBS="${saved_LIBS}"
AH_TEMPLATE(BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD, [Set if boost::program_options::options_description::options() is not available])
if test "$BOOST_PROGRAM_OPTIONS_DESCRIPTION_METHODS" = "old"; then
  AC_DEFINE(BOOST_PROGRAM_OPTIONS_DESCRIPTION_OLD, 1)
fi

AC_MSG_CHECKING([for boost::program_options::validation_error in -lboost_program_options])
saved_LIBS="${LIBS}"
LIBS="${saved_LIBS} -lboost_program_options"
define([boost_error], [AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                                   [boost::program_options::validation_error err(boost::program_options::validation_error::invalid_option, "error");
])])
define([boost_error_old], [AC_LANG_PROGRAM([#include <boost/program_options.hpp>],
                                   [boost::program_options::validation_error err("error");
])])
AC_LINK_IFELSE(boost_error,
               [AC_MSG_RESULT([yes (Boost >= 1.42)])
                BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR="current"],
LIBS="${saved_LIBS} -lboost_program_options"
[AC_LINK_IFELSE(boost_error_old,
               [AC_MSG_RESULT([yes (Boost < 1.42)])
                BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR="old"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_program_options (Boost C++ Libraries) is not installed, but is required by schroot])])])
LIBS="${saved_LIBS}"
AH_TEMPLATE(BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR_OLD, [Set if boost::program_options::validation error uses old construction semantics])
if test "$BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR" = "old"; then
  AC_DEFINE(BOOST_PROGRAM_OPTIONS_VALIDATION_ERROR_OLD, 1)
fi

AC_MSG_CHECKING([for boost::regex in -lboost_regex])
saved_LIBS="${LIBS}"
LIBS="${saved_LIBS} -lboost_regex"
define([testprog], [AC_LANG_PROGRAM([#include <boost/regex.hpp>],
                                   [boost::regex("^foo[bar]$")])])
AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_regex"],
LIBS="${saved_LIBS} -lboost_regex-mt"
[AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_LIBS="${BOOST_LIBS} -lboost_regex-mt"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_regex (Boost C++ Libraries) is not installed, but is required by schroot])])])
LIBS="${saved_LIBS}"

AC_MSG_CHECKING([for boost::filesystem in -lboost_filesystem])
saved_LIBS="${LIBS}"
LIBS="${saved_LIBS} -lboost_filesystem"
define([testprog], [AC_LANG_PROGRAM([#include <boost/filesystem.hpp>],
                                   [boost::filesystem::is_directory("/")])])
AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_FILESYSTEM_LIBS="-lboost_filesystem"],
LIBS="${saved_LIBS} -lboost_filesystem-mt"
[AC_LINK_IFELSE(testprog,
               [AC_MSG_RESULT([yes])
	        BOOST_FILESYSTEM_LIBS="-lboost_filesystem-mt"],
               [AC_MSG_RESULT([no])
	        AC_MSG_FAILURE([libboost_filesystem (Boost C++ Libraries) is not installed, but is required by schroot])])])
LIBS="${saved_LIBS}"

AC_SUBST([BOOST_LIBS])
AC_SUBST([BOOST_FILESYSTEM_LIBS])

AC_MSG_CHECKING([for __gnu_cxx::stdio_filebuf in libstdc++])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <ext/stdio_filebuf.h>
#include <unistd.h>],
                                [__gnu_cxx::stdio_filebuf<char> fdbuf(STDOUT_FILENO, std::ios::out)])],
               [AC_MSG_RESULT([yes])],
               [AC_MSG_RESULT([no])
                AC_MSG_FAILURE([__gnu_cxx::stdio_filebuf (GNU libstdc++) is not installed, but is required by schroot])])

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

SCHROOT_FILE_UNPACK_DIR="${PACKAGE_LOCALSTATE_DIR}/lib/${PACKAGE}/unpack"
AC_SUBST([SCHROOT_FILE_UNPACK_DIR])
AH_TEMPLATE(SCHROOT_FILE_UNPACK_DIR, [schroot file unpack directory])
AC_DEFINE_UNQUOTED(SCHROOT_FILE_UNPACK_DIR, ["$SCHROOT_FILE_UNPACK_DIR"])

SCHROOT_OVERLAY_DIR="${PACKAGE_LOCALSTATE_DIR}/lib/${PACKAGE}/union/overlay"
AC_SUBST([SCHROOT_OVERLAY_DIR])
AH_TEMPLATE(SCHROOT_OVERLAY_DIR, [schroot overlay directory])
AC_DEFINE_UNQUOTED(SCHROOT_OVERLAY_DIR, ["$SCHROOT_OVERLAY_DIR"])

SCHROOT_UNDERLAY_DIR="${PACKAGE_LOCALSTATE_DIR}/lib/${PACKAGE}/union/underlay"
AC_SUBST([SCHROOT_UNDERLAY_DIR])
AH_TEMPLATE(SCHROOT_UNDERLAY_DIR, [schroot underlay directory])
AC_DEFINE_UNQUOTED(SCHROOT_UNDERLAY_DIR, ["$SCHROOT_UNDERLAY_DIR"])

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

SCHROOT_CONF_CHROOT_D="${PACKAGE_SYSCONF_DIR}/chroot.d"
AC_SUBST([SCHROOT_CONF_CHROOT_D])

AH_TEMPLATE(SCHROOT_CONF_CHROOT_D, [schroot chroot directory])
AC_DEFINE_UNQUOTED(SCHROOT_CONF_CHROOT_D, ["$SCHROOT_CONF_CHROOT_D"])

SCHROOT_CONF_SETUP_D="${PACKAGE_SYSCONF_DIR}/setup.d"
AC_SUBST([SCHROOT_CONF_SETUP_D])

AH_TEMPLATE(SCHROOT_CONF_SETUP_D, [schroot chroot setup directory])
AC_DEFINE_UNQUOTED(SCHROOT_CONF_SETUP_D, ["$SCHROOT_CONF_SETUP_D"])

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
AC_CONFIG_FILES([bin/schroot-listmounts/Makefile])
AC_CONFIG_FILES([bin/schroot-mount/Makefile])
AC_CONFIG_FILES([bin/schroot-releaselock/Makefile])
AC_CONFIG_FILES([bin/dchroot/Makefile])
AC_CONFIG_FILES([bin/dchroot-dsa/Makefile])
AC_CONFIG_FILES([bin/csbuild/Makefile])
AC_CONFIG_FILES([contrib/Makefile])
AC_CONFIG_FILES([contrib/setup.d/Makefile])
AC_CONFIG_FILES([etc/Makefile])
AC_CONFIG_FILES([etc/pam/Makefile])
AC_CONFIG_FILES([etc/setup.d/Makefile])
AC_CONFIG_FILES([etc/bash_completion/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([man/schroot.1])
AC_CONFIG_FILES([man/schroot-setup.5])
AC_CONFIG_FILES([man/schroot.conf.5])
AC_CONFIG_FILES([man/schroot-script-config.5])
AC_CONFIG_FILES([man/schroot-faq.7])
AC_CONFIG_FILES([man/dchroot.1])
AC_CONFIG_FILES([man/dchroot-dsa.1])
AC_CONFIG_FILES([man/csbuild.1])
AC_CONFIG_FILES([scripts/po-notify])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([Makefile])
dnl Output the generated config.status script.
AC_OUTPUT
