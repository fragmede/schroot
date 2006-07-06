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

AM_CXXFLAGS = $(SCHROOT_CFLAGS) $(CPPUNIT_CFLAGS) -DTESTDATADIR='"./testdata"' -pedantic -Wall -Wcast-align -Wwrite-strings -Wswitch-default -Wcast-qual -Wunused-variable -Wredundant-decls -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast -Woverloaded-virtual -fstrict-aliasing
#  -Weffc++ causes too many warnings in standard headers; -Wextra is not
#  supported by GCC 3.4.

if USE_UNIT_TESTS
noinst_LTLIBRARIES = libtest.la

noinst_PROGRAMS = 		\
	sbuild-chroot		\
	sbuild-chroot-config	\
	sbuild-environment	\
	sbuild-dirstream	\
	sbuild-keyfile		\
	sbuild-lock		\
	sbuild-log		\
	sbuild-nostream		\
	sbuild-parse-value	\
	sbuild-personality	\
	sbuild-run-parts	\
	sbuild-util

# sbuild-lock is excluded, because it is timing dependent and can fail
# randomly on slow or heavily-loaded systems.
TESTS = setup-test-data		\
	sbuild-chroot		\
	sbuild-chroot-config	\
	sbuild-dirstream	\
	sbuild-environment	\
	sbuild-keyfile		\
	sbuild-log		\
	sbuild-nostream		\
	sbuild-parse-value	\
	sbuild-personality	\
	sbuild-run-parts	\
	sbuild-util		\
	cleanup-test-data
endif

libtest_la_SOURCES = 	\
	test-helpers.h	\
	testmain.cc
libtest_la_LIBADD = $(top_builddir)/sbuild/libsbuild.la $(CPPUNIT_LIBS)

sbuild_chroot_SOURCES = 		\
	sbuild-chroot.cc		\
	sbuild-chroot-plain.cc		\
	sbuild-chroot-file.cc		\
	sbuild-chroot-block-device.cc	\
	sbuild-chroot-lvm-snapshot.cc	\
	test-sbuild-chroot.h
sbuild_chroot_LDADD =  libtest.la

sbuild_chroot_config_SOURCES = sbuild-chroot-config.cc
sbuild_chroot_config_LDADD = libtest.la

sbuild_dirstream_SOURCES = sbuild-dirstream.cc
sbuild_dirstream_LDADD = libtest.la

sbuild_environment_SOURCES = sbuild-environment.cc
sbuild_environment_LDADD = libtest.la

sbuild_keyfile_SOURCES = sbuild-keyfile.cc
sbuild_keyfile_LDADD = libtest.la

sbuild_lock_SOURCES = sbuild-lock.cc
sbuild_lock_LDADD = libtest.la

sbuild_log_SOURCES = sbuild-log.cc
sbuild_log_LDADD = libtest.la

sbuild_nostream_SOURCES = sbuild-nostream.cc
sbuild_nostream_LDADD = libtest.la

sbuild_parse_value_SOURCES = sbuild-parse-value.cc
sbuild_parse_value_LDADD = libtest.la

sbuild_personality_SOURCES = sbuild-personality.cc
sbuild_personality_LDADD = libtest.la

sbuild_run_parts_SOURCES = sbuild-run-parts.cc
sbuild_run_parts_LDADD = libtest.la

sbuild_util_SOURCES = sbuild-util.cc
sbuild_util_LDADD = libtest.la

EXTRA_DIST = 			\
	keyfile.ex1		\
	config.ex1 		\
	config.ex2/file		\
	config.ex2/empty	\
	config.ex2/experimental	\
	config.ex2/sarge	\
	config.ex2/sid		\
	config.ex2/woody	\
	run-parts.ex1/10test1	\
	run-parts.ex1/20test2	\
	run-parts.ex1/30test3	\
	run-parts.ex2		\
	run-parts.ex3/50invalid	\
	setup-test-data		\
	cleanup-test-data

clean-local:
	$(srcdir)/cleanup-test-data