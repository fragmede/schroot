# schroot Makefile template
#
#
# Copyright © 2004-2007  Roger Leigh <rleigh@debian.org>
#
# schroot is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# schroot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307  USA
#
#####################################################################

include $(top_srcdir)/scripts/global.mk

if MAINTAINER_MODE
all-local: schroot
endif

if USE_DOXYGEN
schroot: schroot-stamp
else
schroot:
endif

schroot-stamp: schroot.dox $(top_srcdir)/configure
	$(RM) -rf schroot/html
	$(DOXYGEN) schroot.dox
	touch $@

PS_DEPS = \
	$(top_builddir)/schroot/schroot.1		\
	$(top_builddir)/schroot/schroot.conf.5		\
	$(top_builddir)/schroot/schroot-setup.5		\
	$(top_builddir)/dchroot/dchroot.1		\
	$(top_builddir)/dchroot-dsa/dchroot-dsa.1

schroot.ps: $(PS_DEPS)
	tbl $(PS_DEPS) | groff -T ps -man > $@

schroot.pdf: schroot.ps
	ps2pdf $< $@

CLEANFILES =		\
	schroot.log	\
	schroot.ps	\
	schroot.pdf

maintainer-clean-local:
	$(RM) -r schroot

MAINTAINERCLEANFILES = \
	schroot-stamp

EXTRA_DIST =		\
	schroot		\
	schroot-stamp

.PHONY: schroot
