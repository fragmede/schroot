# schroot Makefile template
#
#
# Copyright © 2004-2009  Roger Leigh <rleigh@debian.org>
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

if BUILD_DCHROOT
dchroot_mans = dchroot.1
endif

if BUILD_DCHROOT_DSA
dchroot_dsa_mans = dchroot-dsa.1
endif

if BUILD_CSBUILD
csbuild_mans = csbuild.1
endif

man_MANS =			\
	schroot.1		\
	schroot-setup.5		\
	schroot.conf.5		\
	schroot-script-config.5	\
	$(dchroot_mans)		\
	$(dchroot_dsa_mans)	\
	$(csbuild_mans)
