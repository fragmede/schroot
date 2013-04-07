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

#include <sbuild/chroot/plain.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include "format-detail.h"
#include "lock.h"

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

    plain::plain ():
      directory_base()
    {
      set_run_setup_scripts(false);

      remove_facet<facet::session_clonable>();
    }

    plain::~plain ()
    {
    }

    chroot::ptr
    plain::clone () const
    {
      return ptr(new plain(*this));
    }

    chroot::ptr
    plain::clone_session (std::string const& session_id,
                          std::string const& alias,
                          std::string const& user,
                          bool               root) const
    {
      return ptr();
    }

    chroot::ptr
    plain::clone_source () const
    {
      return ptr();
    }

    std::string const&
    plain::get_chroot_type () const
    {
      static const std::string type("plain");

      return type;
    }

    std::string
    plain::get_path () const
    {
      return get_directory();
    }

    void
    plain::setup_lock (setup_type type,
                       bool       lock,
                       int        status)
    {
      /* By default, plain chroots do no locking. */
    }

    chroot::session_flags
    plain::get_session_flags (chroot const& chroot) const
    {
      return SESSION_NOFLAGS;
    }

  }
}
