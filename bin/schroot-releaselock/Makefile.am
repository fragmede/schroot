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

if BUILD_BLOCKDEV
pkglibexec_PROGRAMS += bin/schroot-releaselock/schroot-releaselock
endif

bin_schroot_releaselock_schroot_releaselock_SOURCES = 		\
	bin/schroot-releaselock/schroot-releaselock-main.h	\
	bin/schroot-releaselock/schroot-releaselock-main.cc	\
	bin/schroot-releaselock/schroot-releaselock-options.h	\
	bin/schroot-releaselock/schroot-releaselock-options.cc	\
	bin/schroot-releaselock/schroot-releaselock.cc
bin_schroot_releaselock_schroot_releaselock_LDADD = $(top_builddir)/bin/schroot-base/libschroot-base.la
