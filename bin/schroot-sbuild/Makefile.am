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

schroot_sbuild = bin/schroot-sbuild/schroot-sbuild

bin_PROGRAMS += $(schroot_sbuild)

bin_schroot_sbuild_schroot_sbuild_SOURCES =		\
	bin/schroot-sbuild/session.h	\
	bin/schroot-sbuild/session.cc	\
	bin/schroot-sbuild/main.h	\
	bin/schroot-sbuild/main.cc	\
	bin/schroot-sbuild/schroot-sbuild.cc
bin_schroot_sbuild_schroot_sbuild_LDADD = $(top_builddir)/bin/schroot/libschroot-all.la

install-exec-hook::
# Install setuid root.
	if [ -f "$(DESTDIR)$(bindir)/schroot-sbuild" ]; then \
	  chmod 4755 "$(DESTDIR)$(bindir)/schroot-sbuild"; \
	fi
