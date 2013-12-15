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

SUBDIRS = historical

if BUILD_DOXYGEN
doc: sbuild schroot
else
doc:
endif

schroot: schroot.log
sbuild: sbuild.log

sbuild.log: sbuild.dox $(top_srcdir)/configure
	$(RM) -rf sbuild/html
	$(DOXYGEN) sbuild.dox
	touch $@
	@UNDOCUMENTED=$$(cat $@); \
	if [ -n "$$UNDOCUMENTED" ]; then \
	  echo "---------- UNDOCUMENTED CODE ----------"; \
	  echo "$$UNDOCUMENTED"; \
	fi

schroot.log: sbuild.log schroot.dox $(top_srcdir)/configure
	$(RM) -rf schroot/html
	$(DOXYGEN) schroot.dox
	touch $@
	@UNDOCUMENTED=$$(cat $@); \
	if [ -n "$$UNDOCUMENTED" ]; then \
	  echo "---------- UNDOCUMENTED CODE ----------"; \
	  echo "$$UNDOCUMENTED"; \
	fi


CLEANFILES =		\
	sbuild.log	\
	schroot.log

clean-local:
	$(RM) -r sbuild
	$(RM) -r schroot

EXTRA_DIST =			\
	CMakeLists.txt		\
	sbuild.dox.cmake	\
	schroot.dox.cmake

.PHONY: doc sbuild schroot
