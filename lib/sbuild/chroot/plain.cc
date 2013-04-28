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
#include <sbuild/chroot/facet/factory.h>
#include <sbuild/chroot/facet/storage.h>
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
      chroot()
    {
      add_facet(std::dynamic_pointer_cast<facet::storage>(facet::factory::create("plain")));
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

  }
}
