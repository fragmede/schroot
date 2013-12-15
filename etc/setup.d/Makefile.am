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

schroot_setupdata_DATA =	\
	common-config		\
	common-data		\
	common-functions

schroot_sysconf_setup_SCRIPTS =	\
	00check			\
	05file			\
	05btrfs			\
	05lvm			\
	05union			\
	10mount			\
	15binfmt		\
	15killprocs		\
	20copyfiles		\
	20nssdatabases		\
	50chrootname		\
	70services

install-exec-hook:
	$(MKDIR_P) "$(DESTDIR)$(schroot_sysconf_setupdir)"
	ln -sf "00check" "$(DESTDIR)$(schroot_sysconf_setupdir)/99check"

EXTRA_DIST =					\
	$(schroot_setupdata_DATA)		\
	$(schroot_sysconf_setup_SCRIPTS)	\
	CMakeLists.txt
