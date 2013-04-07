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

#include "chroot-directory.h"
#include "chroot-facet-session-clonable.h"
#include "chroot-facet-source-clonable.h"
#ifdef SBUILD_FEATURE_UNION
#include "chroot-facet-union.h"
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

chroot_directory::chroot_directory ():
  chroot_directory_base()
{
#ifdef SBUILD_FEATURE_UNION
  add_facet(chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}

chroot_directory::chroot_directory (const chroot_directory& rhs):
  chroot_directory_base(rhs)
{
}

#ifdef SBUILD_FEATURE_BTRFSSNAP
chroot_directory::chroot_directory (const chroot_btrfs_snapshot& rhs):
  chroot_directory_base(rhs)
{
#ifdef SBUILD_FEATURE_UNION
  if (!get_facet<chroot_facet_union>())
    add_facet(chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION

  set_directory(rhs.get_source_subvolume());
}
#endif // SBUILD_FEATURE_BTRFSSNAP

chroot_directory::~chroot_directory ()
{
}

chroot::chroot::ptr
chroot_directory::clone () const
{
  return ptr(new chroot_directory(*this));
}

chroot::chroot::ptr
chroot_directory::clone_session (std::string const& session_id,
                                 std::string const& alias,
                                 std::string const& user,
                                 bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_directory(*this));
  psess->clone_session_setup(*this, session, session_id, alias, user, root);

  return session;
}

chroot::chroot::ptr
chroot_directory::clone_source () const
{
  ptr clone(new chroot_directory(*this));

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(*this, clone);

  return clone;
}

std::string
chroot_directory::get_path () const
{
  return get_mount_location();
}

void
chroot_directory::setup_env (chroot const& chroot,
                             environment& env) const
{
  chroot_directory_base::setup_env(chroot, env);
}

std::string const&
chroot_directory::get_chroot_type () const
{
  static const std::string type("directory");

  return type;
}

void
chroot_directory::setup_lock (chroot::setup_type type,
                              bool               lock,
                              int                status)
{
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

chroot::chroot::session_flags
chroot_directory::get_session_flags (chroot const& chroot) const
{
  return SESSION_NOFLAGS;
}

void
chroot_directory::get_details (chroot const&  chroot,
                               format_detail& detail) const
{
  chroot_directory_base::get_details(chroot, detail);
}

void
chroot_directory::get_used_keys (string_list& used_keys) const
{
  chroot_directory_base::get_used_keys(used_keys);
}

void
chroot_directory::get_keyfile (chroot const& chroot,
                               keyfile&      keyfile) const
{
  chroot_directory_base::get_keyfile(chroot, keyfile);
}

void
chroot_directory::set_keyfile (chroot& chroot,
                               keyfile const& keyfile)
{
  chroot_directory_base::set_keyfile(chroot, keyfile);
}
