# schroot Makefile template
#
#
# Copyright © 2004-2010  Roger Leigh <rleigh@debian.org>
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

SUBDIRS = pam setup.d bash_completion

schroot_sysconf_DATA = 		\
	schroot.conf

PROFILES =		\
	buildd		\
	default		\
	desktop		\
	minimal		\
	sbuild

profiles: profiles-stamp
profiles-stamp: genprofile
	$(MKDIR_P) profiles
	for profile in $(PROFILES); do 					\
	  $(srcdir)/genprofile "$$profile" "$(PLATFORM)" "$(srcdir)/profile-templates" "profiles/$$profile";	\
	done
	touch $@

all-local: profiles

install-data-hook:
	for profile in $(PROFILES); do 						\
	  $(MKDIR_P) "$(DESTDIR)$(schroot_sysconfdir)/$$profile";			\
	  for file in "profiles/$$profile"/*; do						\
	    $(INSTALL_DATA) "$$file" "$(DESTDIR)$(schroot_sysconfdir)/$$profile";	\
	  done;									\
	done

clean-local:
	$(RM) -rf profiles
	$(RM) -f profiles-stamp

EXTRA_DIST =			\
	$(schroot_sysconf_DATA)	\
	genprofile		\
	profile-templates	\
	CMakeLists.txt

.PHONY: profiles
