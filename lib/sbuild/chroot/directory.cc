/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <config.h>

#include <sbuild/chroot/directory.h>
#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/btrfs-snapshot.h>
#include <sbuild/chroot/facet/directory.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION
#include "format-detail.h"
#include "lock.h"

#include <cassert>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {

    directory::directory ():
      chroot()
    {
      add_facet(std::dynamic_pointer_cast<facet::storage>(facet::factory::create("directory")));
    }

    directory::directory (const directory& rhs):
      chroot(rhs)
    {
    }

#ifdef SBUILD_FEATURE_BTRFSSNAP
    directory::directory (const btrfs_snapshot& rhs):
      chroot(rhs)
    {
      facet::storage::ptr dir = facet::directory::create(*get_facet_strict<facet::btrfs_snapshot>());
      replace_facet<facet::storage>(dir);
    }
#endif // SBUILD_FEATURE_BTRFSSNAP

    directory::~directory ()
    {
    }

    chroot::ptr
    directory::clone () const
    {
      return ptr(new directory(*this));
    }

    chroot::ptr
    directory::clone_session (std::string const& session_id,
                              std::string const& alias,
                              std::string const& user,
                              bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new directory(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

  }
}
