# schroot Makefile template
#
#
# Copyright © 2004-2005  Roger Leigh <rleigh@debian.org>
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

SUBDIRS = pam session setup

localedir = $(datadir)/locale

AM_CFLAGS = -std=c99
DEFS = -DG_LOG_DOMAIN=\"Schroot\" -DGETTEXT_PACKAGE=\"schroot\" -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = schroot

schroot_public_h_sources =		\
	sbuild-auth.h			\
	sbuild-auth-conv.h		\
	sbuild-auth-conv-tty.h		\
	sbuild-auth-message.h   	\
	sbuild-chroot.h			\
	sbuild-chroot-block-device.h	\
	sbuild-chroot-lvm-snapshot.h	\
	sbuild-chroot-plain.h		\
	sbuild-config.h			\
	sbuild-error.h			\
	sbuild-lock.h			\
	sbuild-session.h

schroot_public_c_sources =		\
	sbuild-auth.c			\
	sbuild-auth-conv.c		\
	sbuild-auth-conv-tty.c		\
	sbuild-auth-message.c   	\
	sbuild-chroot.c			\
	sbuild-chroot-block-device.c	\
	sbuild-chroot-lvm-snapshot.c	\
	sbuild-chroot-plain.c		\
	sbuild-config.c			\
	sbuild-error.c			\
	sbuild-lock.c			\
	sbuild-session.c

schroot_built_sources =		\
	sbuild-marshallers.c	\
	sbuild-marshallers.h	\
	sbuild-typebuiltins.c	\
	sbuild-typebuiltins.h

BUILT_SOURCES = $(schroot_built_sources)

schroot_SOURCES =			\
	$(schroot_public_h_sources)	\
	$(schroot_public_c_sources)	\
	$(schroot_built_sources)	\
	schroot.h			\
	schroot.c

schroot_CFLAGS = $(AM_CFLAGS) $(SCHROOT_CFLAGS)
schroot_LDADD = $(GOBJECT_LIBS) $(UUID_LIBS) $(PAM_LIBS) $(LIBINTL)

if ENABLE_GTK_DOC
noinst_LTLIBRARIES = libschroot.la
libschroot_la_SOURCES = $(schroot_SOURCES)
libschroot_la_CFLAGS = $(schroot_CFLAGS)
libschroot_la_LIBADD = $(schroot_LDADD)
endif

pkgsysconfdir = $(PACKAGE_SYSCONF_DIR)
pkgsysconf_DATA = schroot.conf

man_MANS = schroot.1 schroot-setup.5 schroot.conf.5

sbuild-marshallers.c : sbuild-marshallers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=sbuild_cclosure_marshal > $@.tmp
	echo '/* This file has been automatically generated.  Do not edit.  */' >$@
	echo '#include "'$*.h'"' >>$@
	cat $@.tmp >>$@
	rm -f $@.tmp

sbuild-marshallers.h : sbuild-marshallers.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=sbuild_cclosure_marshal > $@

$(srcdir)/sbuild-typebuiltins.c: $(schroot_public_h_sources) $(typebuiltins_extra_deps) $(GLIB_MKENUMS)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#include \"schroot.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(schroot_public_h_sources) ) > $@

$(srcdir)/sbuild-typebuiltins.h: $(schroot_public_h_sources) $(typebuiltins_extra_deps) $(GLIB_MKENUMS)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#ifndef SBUILD_TYPE_BUILTINS_H\n#define SBUILD_TYPE_BUILTINS_H\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define SBUILD_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* SBUILD_TYPE_BUILTINS_H */" \
		$(schroot_public_h_sources) ) > $@


EXTRA_DIST =				\
	$(pkgsysconf_DATA)		\
	$(schroot_built_sources)	\
	sbuild-marshallers.list

install-exec-hook:
# Install setuid root.
	chmod 4755 $(DESTDIR)$(bindir)/schroot

install-data-hook:
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_LOCK_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_MOUNT_DIR)
	$(MKINSTALLDIRS) $(DESTDIR)$(SCHROOT_SESSION_DIR)