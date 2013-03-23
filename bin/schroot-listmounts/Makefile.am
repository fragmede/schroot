# schroot Makefile template
#
#
# Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>
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

pkglibexec_PROGRAMS += bin/schroot-listmounts/schroot-listmounts

bin_schroot_listmounts_schroot_listmounts_SOURCES =		\
	bin/schroot-listmounts/schroot-listmounts-main.h	\
	bin/schroot-listmounts/schroot-listmounts-main.cc	\
	bin/schroot-listmounts/schroot-listmounts-options.h	\
	bin/schroot-listmounts/schroot-listmounts-options.cc	\
	bin/schroot-listmounts/schroot-listmounts.cc
bin_schroot_listmounts_schroot_listmounts_LDADD = \
	$(top_builddir)/bin/schroot-base/libschroot-base.la
