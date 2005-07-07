/* sbuild-error - sbuild error handling
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * serror is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * serror is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

/**
 * SECTION:sbuild-error
 * @short_description: GError typebuiltins
 * @title: SbuildError
 *
 * GError is not registered as a GType.  Here, it is registered as a
 * GBoxed boxed type, and GError* is also boxed, for use as an out
 * parameter in callbacks.  Hopefully this will someday be included in
 * libgobect itself.  See glib bug #300610
 * (http://bugzilla.gnome.org/show_bug.cgi?id=300610)
 */

#include <config.h>

#include "sbuild-error.h"


GType
sbuild_error_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    type = g_boxed_type_register_static ("GError",
                                         (GBoxedCopyFunc) g_error_copy,
                                         (GBoxedFreeFunc) g_error_free);

  return type;
}

static SbuildErrorPointer *
sbuild_error_pointer_copy (const SbuildErrorPointer *errorptr)
{
  if (errorptr == NULL)
    return NULL;

  SbuildErrorPointer *copy =
    (SbuildErrorPointer *) g_memdup (errorptr, sizeof (SbuildErrorPointer));

  return copy;
}

static void
sbuild_error_pointer_free (SbuildErrorPointer  *errorptr)
{
  if (errorptr == NULL)
    return;

  g_free (errorptr);
}

GType
sbuild_error_pointer_get_type (void)
{
  static GType type = 0;

  if (type == 0)
    type = g_boxed_type_register_static
      ("SbuildErrorPointer",
       (GBoxedCopyFunc) sbuild_error_pointer_copy,
       (GBoxedFreeFunc) sbuild_error_pointer_free);

  return type;
}


/*
 * Local Variables:
 * mode:C
 * End:
 */
