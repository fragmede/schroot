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

noinst_LTLIBRARIES += bin/schroot-base/libschroot-base.la

bin_schroot_base_libschroot_base_la_SOURCES =	\
	bin/schroot-base/main.h			\
	bin/schroot-base/main.cc		\
	bin/schroot-base/option-action.h	\
	bin/schroot-base/option-action.cc	\
	bin/schroot-base/options.h		\
	bin/schroot-base/options.cc		\
	bin/schroot-base/run.h
bin_schroot_base_libschroot_base_la_LIBADD = $(top_builddir)/sbuild/libsbuild.la
