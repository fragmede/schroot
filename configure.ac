#                                               -*- Autoconf -*-
# Copyright © 2004-2005  Roger Leigh <rleigh@debian.org>
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
dnl The minimum version of autoconf required.
AC_PREREQ(2.59)
dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([schroot], [0.1.2], [rleigh@debian.org])
dnl For safety, check we are in the right directory by
dnl checking for a known unique file.
AC_CONFIG_SRCDIR([schroot/sbuild-session.c])
dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])
dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE(1.9 gnu check-news dist-bzip2 no-dist-gzip)

AC_MSG_CHECKING([whether to enable debugging messages])
AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [Enable debugging messages])],
              [ case "${enableval}" in
                yes) enable_debug="yes" ; AC_MSG_RESULT([yes]) ;;
                no)  enable_debug="no"  ; AC_MSG_RESULT([no]) ;;
                unset)  enable_debug="no"  ; AC_MSG_RESULT([no]) ;;
                *) AC_MSG_RESULT([unknown])
                   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
                esac])
if test "$enable_debug" = "yes"; then
  AC_DEFINE_UNQUOTED([SBUILD_DEBUG], [1], [Enable debugging])
fi

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
dnl AC_PROG_CC_C99
AC_PROG_LIBTOOL
AM_GNU_GETTEXT_VERSION([0.14.5])
AM_GNU_GETTEXT([external])
AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])
GTK_DOC_CHECK([1.2])
# Checks for libraries.
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4.0])

SCHROOT_CFLAGS="$GOBJECT_CFLAGS"
AC_SUBST([SCHROOT_CFLAGS])

# Checks for header files (none at the moment).

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions (none at the moment).
AC_CHECK_LIB([pam], [pam_authenticate],
             [PAM_LIBS="-lpam"],
	     [AC_MSG_ERROR([Linux-PAM is not installed, but is required by schroot])])
AC_CHECK_LIB([pam_misc], [pam_getenvlist],
             [PAM_LIBS="${PAM_LIBS} -lpam_misc"],
             [AC_MSG_ERROR([Linux-PAM is not installed, but is required by schroot])])
AC_SUBST([PAM_LIBS]) 

dnl Set PACKAGE_LOCALE_DIR in config.h
AH_TEMPLATE(PACKAGE_LOCALE_DIR,, [Package locale directory])
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ["${ac_default_prefix}/share/locale"])
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ["${prefix}/share/locale"])
fi

dnl Set PACKAGE_DATA_DIR in config.h.
AH_TEMPLATE(PACKAGE_DATA_DIR,, [Package data directory])
if test "x${datadir}" = 'x${prefix}/share'; then
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
AH_TEMPLATE(PACKAGE_LIB_DIR,, [Package lib directory])
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

AH_TEMPLATE(SCHROOT_DATADIR,, [Package  data directory])
SCHROOT_DATADIR="${PACKAGE_DATA_DIR}/${PACKAGE_VERSION}"
AC_DEFINE_UNQUOTED(SCHROOT_DATADIR, ["$SCHROOT_DATADIR"], )

AH_TEMPLATE(SCHROOT_MODULEDIR,, [Package module directory])
SCHROOT_MODULEDIR="${PACKAGE_LIB_DIR}/${PACKAGE_VERSION}/modules"
AC_DEFINE_UNQUOTED(SCHROOT_MODULEDIR, ["$SCHROOT_MODULEDIR"])


dnl Configure which files to generate.
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/schroot/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([schroot/Makefile])
AC_CONFIG_FILES([schroot/pam/Makefile])
AC_CONFIG_FILES([schroot/schroot.1])
AC_CONFIG_FILES([schroot/schroot.conf.5])
AC_CONFIG_FILES([Makefile])
dnl Output the generated config.status script.
AC_OUTPUT
