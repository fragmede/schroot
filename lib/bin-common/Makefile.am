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

noinst_LTLIBRARIES += lib/schroot-base/libschroot-base.la

lib_schroot_base_libschroot_base_la_SOURCES =	\
	lib/schroot-base/main.h			\
	lib/schroot-base/main.cc		\
	lib/schroot-base/option-action.h	\
	lib/schroot-base/option-action.cc	\
	lib/schroot-base/options.h		\
	lib/schroot-base/options.cc		\
	lib/schroot-base/run.h
lib_schroot_base_libschroot_base_la_LIBADD = $(top_builddir)/lib/sbuild/libsbuild.la
