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

ACLOCAL_AMFLAGS=-I m4

include $(top_srcdir)/scripts/global.mk
include $(top_srcdir)/scripts/git-dist.mk

SUBDIRS = sbuild bin test etc contrib man doc po

dist-hook:
# Remove junk from the generated tarball.
	$(FIND) $(distdir) -name '.*' -print | $(XARGS) xargs $(RM) -rf
	$(FIND) $(distdir) -name '#*' -print | $(XARGS) $(RM) -rf
	$(FIND) $(distdir) -name '*~' -print | $(XARGS) $(RM) -rf

# Generate ChangeLog
	if [ -d .git ]; then			\
	  git log > $(distdir)/ChangeLog;	\
	fi

EXTRA_DIST =				\
	bootstrap			\
	debian				\
	scripts/schroot_release.m4.in	\
	m4				\
	HACKING				\
	VERSION				\
	CMakeLists.txt			\
	cmake/boost-checks.cmake	\
	cmake/CompilerChecks.cmake	\
	cmake/GitRelease.cmake		\
	cmake/GitReleasePolicy.cmake	\
	cmake/GitVersion.cmake		\
	cmake/regex-checks.cmake	\
	po/CMakeLists.txt		\
	config.h.cmake

doc:
	$(MAKE) -C doc doc

ps:
	$(MAKE) -C man ps

pdf:
	$(MAKE) -C man pdf

.PHONY: doc ps pdf
