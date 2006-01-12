# schroot Makefile template
#
#
# Copyright © 2004-2006  Roger Leigh <rleigh@debian.org>
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
#####################################################################

AM_CXXFLAGS = $(SCHROOT_CFLAGS) $(CPPUNIT_CFLAGS) -DSRCDIR='"$(srcdir)"' -pedantic -Wall -Wcast-align -Wextra -Wwrite-strings -Wswitch-default -Wcast-qual -Wunused-variable -Wredundant-decls -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast -Woverloaded-virtual -fstrict-aliasing
#  -Weffc++ causes too many warnings in standard headers.

if USE_UNIT_TESTS
noinst_LTLIBRARIES = libtest.la

noinst_PROGRAMS = 		\
	sbuild-chroot		\
	sbuild-environment	\
	sbuild-keyfile		\
	sbuild-util

TESTS = $(noinst_PROGRAMS)
endif

libtest_la_SOURCES = 	\
	test-helpers.h	\
	testmain.cc
libtest_la_LIBADD = ../schroot/libsbuild.la $(CPPUNIT_LIBS)

sbuild_chroot_SOURCES = 		\
	sbuild-chroot.cc		\
	sbuild-chroot-plain.cc		\
	sbuild-chroot-block-device.cc	\
	sbuild-chroot-lvm-snapshot.cc	\
	test-sbuild-chroot.h
sbuild_chroot_LDADD =  libtest.la

sbuild_environment_SOURCES = sbuild-environment.cc
sbuild_environment_LDADD = libtest.la

sbuild_keyfile_SOURCES = sbuild-keyfile.cc
sbuild_keyfile_LDADD = libtest.la

sbuild_util_SOURCES = sbuild-util.cc
sbuild_util_LDADD = libtest.la
