# schroot Makefile template
#
#
# Copyright © 2004-2006  Roger Leigh <rleigh@debian.org>
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

all-local: schroot

if USE_DOXYGEN
schroot: schroot-stamp
else
schroot:
endif

schroot-stamp: schroot.dox $(top_srcdir)/configure
	$(RM) -rf schroot/html
	$(DOXYGEN) schroot.dox
	touch $@

CLEANFILES = \
	schroot.log

maintainer-clean-local:
	$(RM) -r schroot

MAINTAINERCLEANFILES = \
	schroot-stamp

EXTRA_DIST = \
	schroot

.PHONY: schroot
