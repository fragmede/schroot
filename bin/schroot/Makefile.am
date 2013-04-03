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

noinst_LTLIBRARIES +=			\
	bin/schroot/libschroot.la

bin_PROGRAMS += bin/schroot/schroot

bin_schroot_libschroot_la_SOURCES =	\
	bin/schroot/options-base.h	\
	bin/schroot/options-base.cc	\
	bin/schroot/main-base.h		\
	bin/schroot/main-base.cc	\
	bin/schroot/main.h		\
	bin/schroot/main.cc
bin_schroot_libschroot_la_LIBADD = $(top_builddir)/lib/bin-common/libbin-common.la

bin_schroot_schroot_SOURCES =		\
	bin/schroot/schroot.cc		\
	bin/schroot/options.h		\
	bin/schroot/options.cc
bin_schroot_schroot_LDADD = bin/schroot/libschroot.la

install-exec-hook::
# Install setuid root.
	chmod 4755 "$(DESTDIR)$(bindir)/schroot"

install-data-hook::
	$(MKDIR_P) $(DESTDIR)$(schroot_sysconf_chrootdir)
	$(MKDIR_P) $(DESTDIR)$(schroot_mountdir)
	$(MKDIR_P) $(DESTDIR)$(schroot_sessiondir)
	$(MKDIR_P) $(DESTDIR)$(schroot_file_unpackdir)
	$(MKDIR_P) $(DESTDIR)$(schroot_overlaydir)
	$(MKDIR_P) $(DESTDIR)$(schroot_underlaydir)

