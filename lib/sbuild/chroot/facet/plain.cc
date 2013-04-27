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

#include <sbuild/chroot/facet/plain.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include "format-detail.h"
#include "util.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      plain::plain ():
        directory_base()
      {
      }

      plain::~plain ()
      {
      }

      plain::plain (const plain& rhs):
        directory_base(rhs)
      {
      }

      void
      plain::set_chroot (chroot& chroot)
      {
        directory_base::set_chroot(chroot);
        owner->remove_facet<session_clonable>();
      }

      std::string const&
      plain::get_name () const
      {
        static const std::string name("plain");

        return name;
      }

      plain::ptr
      plain::create ()
      {
        return ptr(new plain());
      }

      facet::ptr
      plain::clone () const
      {
        return ptr(new plain(*this));
      }

      std::string
      plain::get_path () const
      {
        return get_directory();
      }

    }
  }
}
