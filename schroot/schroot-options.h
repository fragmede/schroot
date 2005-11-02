/* schroot-options - schroot options parser
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
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

#ifndef SBUILD_SCHROOT_OPTIONS_H
#define SBUILD_SCHROOT_OPTIONS_H

#include "sbuild-session.h"

typedef struct _SchrootOptions SchrootOptions;

struct _SchrootOptions
{
  char                   **chroots;
  char                   **command;
  char                    *user;
  gboolean                 preserve;
  gboolean                 quiet;
  gboolean                 verbose;
  gboolean                 list;
  gboolean                 info;
  gboolean                 all;
  gboolean                 all_chroots;
  gboolean                 all_sessions;
  gboolean                 load_chroots;
  gboolean                 load_sessions;
  gboolean                 version;
  SbuildSessionOperation   session_operation;
  gboolean                 session_force;
};

SchrootOptions *
schroot_options_new (void);

SchrootOptions *
schroot_options_parse (int   argc,
		       char *argv[]);

void
schroot_options_free (SchrootOptions *options);

#endif /* SBUILD_SCHROOT_OPTIONS_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
