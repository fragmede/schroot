# schroot Makefile template
#
#
# Copyright © 2004-2012  Roger Leigh <rleigh@debian.org>
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

# Available translations
LINGUAS=$(shell if [ -n "$(PO4A)" ]; then cat $(srcdir)/po/LINGUAS; fi)
# Actual translatable manual pages (missing pages are not be translated yet)
TRANSMAN = $(shell for MAN in $(foreach LANG,$(LINGUAS),$(addprefix translated/$(LANG)/,$(man_MANS))); do if [ -e "$$MAN.man" ]; then echo $$MAN; fi; done)

translated/%.man: update-po

schroot.ps: $(man_MANS)
	tbl $(man_MANS) | groff -K UTF8 -T ps -man > $@

%.ps: PSMAN=$(addprefix translated/$(patsubst .%,%,$(suffix $(basename %)))/,$(man_MANS))
%.ps: $(TRANSMAN)
	tbl $^ | groff -K UTF8 -T ps -man > $@

%.pdf: %.ps
	ps2pdf $< $@

if BUILD_DCHROOT
dchroot_mans = dchroot.1
endif

if BUILD_DCHROOT_DSA
dchroot_dsa_mans = dchroot-dsa.1
endif

if BUILD_CSBUILD
csbuild_mans = csbuild.1
endif

MAN_FILES = \
	csbuild.1.man \
	dchroot.1.man \
	dchroot-dsa.1.man \
	schroot.1.man \
	schroot.conf.5.man \
	schroot-faq.7.man \
	schroot-script-config.5.man \
	schroot-setup.5.man

EXPAND = \
	config.man \
	$(srcdir)/authors.man \
	$(srcdir)/copyright.man

man_MANS =			\
	schroot.1		\
	schroot-setup.5		\
	schroot.conf.5		\
	schroot-script-config.5	\
	schroot-faq.7		\
	$(dchroot_mans)		\
	$(dchroot_dsa_mans)	\
	$(csbuild_mans)

EXTRA_DIST =			\
	$(MAN_FILES)		\
	authors.man		\
	copyright.man		\
	po			\
	po4a.cfg		\
	CMakeLists.txt		\
	config.man.cmake

clean-local:
	rm -rf translated *.ps *.pdf

CLEANFILES =				\
	config.man			\
	$(basename $(MAN_FILES))	\
	schroot.ps			\
	schroot.pdf

%.1: %.1.man $(EXPAND)
	$(SOELIM) -I $(dir $<) -I "$(srcdir)" $< >$@

%.5: %.5.man $(EXPAND)
	$(SOELIM) -I $(dir $<) -I "$(srcdir)" $< >$@

%.7: %.7.man $(EXPAND)
	$(SOELIM) -I $(dir $<) -I "$(srcdir)" $< >$@

%.1.ps: %.1
	groffer -Tps -man <$< >$@

%.5.ps: %.5
	groffer -Tps -man <$< >$@

%.7.ps: %.7
	groffer -Tps -man <$< >$@

all-local: update-po $(TRANSMAN)

ps: schroot.ps $(addprefix translated/,$(addsuffix /schroot.ps,$(LINGUAS)))

pdf: schroot.pdf $(addprefix translated/,$(addsuffix /schroot.pdf,$(LINGUAS)))

CONFIG =\
.ds RELEASE_DATE $(RELEASE_DATE_S)\n\
.ds VERSION $(VERSION)\n\
.ds SCHROOT_LIBEXEC_DIR $(pkglibexecdir)\n\
.ds SCHROOT_MOUNT_DIR $(schroot_mountdir)\n\
.ds SCHROOT_SESSION_DIR $(schroot_sessiondir)\n\
.ds SCHROOT_FILE_UNPACK_DIR $(schroot_file_unpackdir)\n\
.ds SCHROOT_OVERLAY_DIR $(schroot_overlaydir)\n\
.ds SCHROOT_UNDERLAY_DIR $(schroot_underlaydir)\n\
.ds SCHROOT_SYSCONF_DIR $(schroot_sysconfdir)\n\
.ds SCHROOT_CONF $(SCHROOT_CONF)\n\
.ds SCHROOT_CONF_CHROOT_D $(schroot_sysconf_chrootdir)\n\
.ds SCHROOT_CONF_SETUP_D $(schroot_sysconf_setupdir)\n\
.ds SCHROOT_DATA_DIR $(pkgdatadir)\n\
.ds SCHROOT_SETUP_DATA_DIR $(schroot_setupdatadir)\n\
.ds PACKAGE_LOCALE_DIR $(prefix)/share/locale\n\
.ds DCHROOT_CONF $(DCHROOT_CONF)\n\
.ds CSBUILD_CONF $(CSBUILD_CONF)\n\
.ds PROGRAM schroot

config.man: Makefile
	echo -e "$(CONFIG)" | sed -e 's/^[ ]//' > $@

update-po:
	if [ -n "$(PO4A)" ]; then $(PO4A) --package-name $(PACKAGE) --package-version $(VERSION) --copyright-holder "Roger Leigh <rleigh@debian.org>" "--srcdir=$(abs_top_srcdir)/man" "--destdir=$(abs_top_builddir)/man" "$(abs_top_srcdir)/man/po4a.cfg"; fi

dist-hook: update-po

.PHONY: update-po ps pdf
