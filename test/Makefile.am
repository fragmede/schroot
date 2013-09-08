# Copyright © 2004-2013  Roger Leigh <rleigh@debian.org>
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

if USE_UNIT_TESTS

noinst_PROGRAMS += 				\
	test/sbuild/chroot/chroot		\
	test/sbuild/chroot/config		\
	test/sbuild/standard			\
	test/sbuild/keyfile			\
	test/sbuild/lock			\
	test/sbuild/run-parts			\
	test/bin-common/option-action

# lock is excluded, because it is timing dependent and can fail
# randomly on slow or heavily-loaded systems.
TESTS = test/setup-test-data			\
	test/sbuild/chroot/chroot		\
	test/sbuild/chroot/config		\
	test/sbuild/standard			\
	test/sbuild/keyfile			\
	test/sbuild/run-parts			\
	test/bin-common/option-action		\
	test/cleanup-test-data
endif

if BUILD_BLOCKDEV
test_sbuild_chroot_blockdev_sources =	\
	test/sbuild/chroot/block-device.cc
endif

if BUILD_LOOPBACK
test_sbuild_chroot_loopback_sources =	\
	test/sbuild/chroot/loopback.cc
endif

if BUILD_LVMSNAP
test_sbuild_chroot_lvmsnap_sources =	\
	test/sbuild/chroot/lvm-snapshot.cc
endif

if BUILD_BTRFSSNAP
test_sbuild_chroot_btrfssnap_sources =	\
	test/sbuild/chroot/btrfs-snapshot.cc
endif

if BUILD_PERSONALITY
test_sbuild_personality_sources =	\
	test/sbuild/personality.cc
endif
test_sbuild_chroot_chroot_SOURCES =		\
	test/sbuild/chroot/chroot.h		\
	test/sbuild/chroot/chroot.cc		\
	test/sbuild/chroot/plain.cc		\
	test/sbuild/chroot/custom.cc  		\
	test/sbuild/chroot/file.cc		\
	test/sbuild/chroot/directory.cc		\
	$(test_sbuild_chroot_blockdev_sources)	\
	$(test_sbuild_chroot_lvmsnap_sources)	\
	$(test_sbuild_chroot_btrfssnap_sources)	\
	$(test_sbuild_chroot_loopback_sources)	\
	test/sbuild/chroot/facet/userdata.cc
test_sbuild_chroot_chroot_LDADD =  lib/test/libtest.la

test_sbuild_chroot_config_SOURCES = test/sbuild/chroot/config.cc
test_sbuild_chroot_config_LDADD = lib/test/libtest.la

test_sbuild_standard_SOURCES = 			\
	test/sbuild/environment.cc		\
	test/sbuild/log.cc			\
	test/sbuild/nostream.cc			\
	test/sbuild/parse-value.cc		\
	$(test_sbuild_personality_sources)	\
	test/sbuild/regex.cc			\
	test/sbuild/util.cc
test_sbuild_standard_LDADD = lib/test/libtest.la

test_sbuild_keyfile_SOURCES = test/sbuild/keyfile.cc
test_sbuild_keyfile_LDADD = lib/test/libtest.la

test_sbuild_lock_SOURCES = test/sbuild/lock.cc
test_sbuild_lock_LDADD = lib/test/libtest.la

test_sbuild_run_parts_SOURCES = test/sbuild/run-parts.cc
test_sbuild_run_parts_LDADD = lib/test/libtest.la

test_bin_common_option_action_SOURCES = test/bin-common/option-action.cc
test_bin_common_option_action_LDADD = $(top_builddir)/lib/bin-common/libbin-common.la lib/test/libtest.la

EXTRA_DIST += 					\
	test/keyfile.ex1			\
	test/config.ex1 			\
	test/config.ex2/file			\
	test/config.ex2/empty			\
	test/config.ex2/experimental		\
	test/config.ex2/sarge			\
	test/config.ex2/sid			\
	test/config.ex2/woody			\
	test/config-directory-deprecated.ex	\
	test/config-directory-fail.ex		\
	test/config-directory-valid.ex		\
	test/run-parts.ex1/10test1		\
	test/run-parts.ex1/20test2		\
	test/run-parts.ex1/30test3		\
	test/run-parts.ex2			\
	test/run-parts.ex3/50invalid		\
	test/setup-test-data			\
	test/cleanup-test-data

clean-local::
	$(srcdir)/test/cleanup-test-data
