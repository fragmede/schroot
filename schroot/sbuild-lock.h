/* sbuild-lock - sbuild advisory locking
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

#ifndef SBUILD_LOCK_H
#define SBUILD_LOCK_H

#include <unistd.h>
#include <fcntl.h>

#include <glib.h>

typedef enum
{
  SBUILD_LOCK_SHARED    = F_RDLCK,
  SBUILD_LOCK_EXCLUSIVE = F_WRLCK,
  SBUILD_LOCK_NONE      = F_UNLCK
} SbuildLockType;

void
sbuild_lock_set_lock (int            fd,
		      SbuildLockType lock_type,
		      guint          timeout);

void
sbuild_lock_unset_lock (int fd);


#endif /* SBUILD_LOCK_H */

/*
 * Local Variables:
 * mode:C
 * End:
 */
