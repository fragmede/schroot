# schroot Makefile template
#
#
# Copyright © 2004-2008  Roger Leigh <rleigh@debian.org>
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

include $(top_srcdir)/scripts/global.mk

LOCAL_CXXFLAGS = $(SCHROOT_CFLAGS) $(CPPUNIT_CFLAGS) -DTESTDATADIR='"./testdata"' -I$(top_srcdir)/bin

if USE_UNIT_TESTS
noinst_LTLIBRARIES = libtest.la

noinst_PROGRAMS = 			\
	sbuild-chroot			\
	sbuild-chroot-plain		\
	sbuild-chroot-custom		\
	sbuild-chroot-file		\
	sbuild-chroot-directory		\
	sbuild-chroot-block-device	\
	sbuild-chroot-loopback		\
	sbuild-chroot-lvm-snapshot	\
	sbuild-chroot-btrfs-snapshot	\
	sbuild-chroot-config		\
	sbuild-chroot-facet-userdata	\
	sbuild-environment		\
	sbuild-keyfile			\
	sbuild-lock			\
	sbuild-log			\
	sbuild-nostream			\
	sbuild-parse-value		\
	sbuild-personality		\
	sbuild-regex			\
	sbuild-run-parts		\
	sbuild-util			\
	schroot-base-option-action

# sbuild-lock is excluded, because it is timing dependent and can fail
# randomly on slow or heavily-loaded systems.
TESTS = setup-test-data			\
	sbuild-chroot			\
	sbuild-chroot-plain		\
	sbuild-chroot-custom		\
	sbuild-chroot-file		\
	sbuild-chroot-directory		\
	sbuild-chroot-block-device	\
	sbuild-chroot-loopback		\
	sbuild-chroot-lvm-snapshot	\
	sbuild-chroot-btrfs-snapshot	\
	sbuild-chroot-config		\
	sbuild-chroot-facet-userdata	\
	sbuild-environment		\
	sbuild-keyfile			\
	sbuild-log			\
	sbuild-nostream			\
	sbuild-parse-value		\
	sbuild-personality		\
	sbuild-run-parts		\
	sbuild-regex			\
	sbuild-util			\
	schroot-base-option-action	\
	cleanup-test-data
endif

libtest_la_SOURCES = 	\
	test-helpers.h	\
	testmain.cc
libtest_la_LIBADD = $(top_builddir)/sbuild/libsbuild.la $(CPPUNIT_LIBS)

if BUILD_BLOCKDEV
sbuild_chroot_blockdev_sources =	\
	sbuild-chroot-block-device.cc
endif

if BUILD_LOOPBACK
sbuild_chroot_loopback_sources =	\
	sbuild-chroot-loopback.cc
endif

if BUILD_LVMSNAP
sbuild_chroot_lvmsnap_sources =	\
	sbuild-chroot-lvm-snapshot.cc
endif

if BUILD_BTRFSSNAP
sbuild_chroot_btrfssnap_sources =	\
	sbuild-chroot-btrfs-snapshot.cc
endif

sbuild_chroot_SOURCES =		\
	sbuild-chroot.cc	\
	test-sbuild-chroot.h
sbuild_chroot_LDADD =  libtest.la

sbuild_chroot_plain_SOURCES =	\
	sbuild-chroot-plain.cc	\
	test-sbuild-chroot.h
sbuild_chroot_plain_LDADD =  libtest.la

sbuild_chroot_custom_SOURCES =   \
	sbuild-chroot-custom.cc  \
	test-sbuild-chroot.h
sbuild_chroot_custom_LDADD =  libtest.la

sbuild_chroot_file_SOURCES =	\
	sbuild-chroot-file.cc	\
	test-sbuild-chroot.h
sbuild_chroot_file_LDADD =  libtest.la

sbuild_chroot_directory_SOURCES =	\
	sbuild-chroot-directory.cc	\
	test-sbuild-chroot.h
sbuild_chroot_directory_LDADD =  libtest.la

sbuild_chroot_block_device_SOURCES =		\
	$(sbuild_chroot_blockdev_sources)	\
	test-sbuild-chroot.h
sbuild_chroot_block_device_LDADD =  libtest.la

sbuild_chroot_lvm_snapshot_SOURCES =		\
	$(sbuild_chroot_lvmsnap_sources)	\
	test-sbuild-chroot.h
sbuild_chroot_lvm_snapshot_LDADD =  libtest.la

sbuild_chroot_btrfs_snapshot_SOURCES =		\
	$(sbuild_chroot_btrfssnap_sources)	\
	test-sbuild-chroot.h
sbuild_chroot_btrfs_snapshot_LDADD =  libtest.la

sbuild_chroot_loopback_SOURCES =		\
	$(sbuild_chroot_loopback_sources)	\
	test-sbuild-chroot.h
sbuild_chroot_loopback_LDADD =  libtest.la

sbuild_chroot_facet_userdata_SOURCES =		\
	sbuild-chroot-facet-userdata.cc		\
	test-sbuild-chroot.h
sbuild_chroot_facet_userdata_LDADD =  libtest.la

sbuild_chroot_config_SOURCES = sbuild-chroot-config.cc
sbuild_chroot_config_LDADD = libtest.la

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

sbuild_regex_SOURCES = sbuild-regex.cc
sbuild_regex_LDADD = libtest.la

sbuild_run_parts_SOURCES = sbuild-run-parts.cc
sbuild_run_parts_LDADD = libtest.la

sbuild_util_SOURCES = sbuild-util.cc
sbuild_util_LDADD = libtest.la

schroot_base_option_action_SOURCES = schroot-base-option-action.cc
schroot_base_option_action_LDADD = $(top_builddir)/bin/schroot-base/libschroot-base.la libtest.la

EXTRA_DIST = 				\
	keyfile.ex1			\
	config.ex1 			\
	config.ex2/file			\
	config.ex2/empty		\
	config.ex2/experimental		\
	config.ex2/sarge		\
	config.ex2/sid			\
	config.ex2/woody		\
	config-directory-deprecated.ex	\
	config-directory-fail.ex	\
	config-directory-valid.ex	\
	run-parts.ex1/10test1		\
	run-parts.ex1/20test2		\
	run-parts.ex1/30test3		\
	run-parts.ex3/50invalid		\
	setup-test-data			\
	cleanup-test-data		\
	CMakeLists.txt			\
	setup-test-data.cmake		\
	cleanup-test-data.cmake

clean-local:
	$(srcdir)/cleanup-test-data
