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

#include <sbuild/chroot/file.h>
#include <sbuild/chroot/facet/file.h>
#include <sbuild/chroot/facet/session.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#include "format-detail.h"
#include "lock.h"

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

    file::file ():
      chroot()
    {
      add_facet(facet::file::create());
    }

    file::file (const file& rhs):
      chroot(rhs)
    {
    }

    file::~file ()
    {
    }

    chroot::chroot::ptr
    file::clone () const
    {
      return ptr(new file(*this));
    }

    chroot::chroot::ptr
    file::clone_session (std::string const& session_id,
                         std::string const& alias,
                         std::string const& user,
                         bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new file(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    file::clone_source () const
    {
      file *clone_file = new file(*this);
      ptr clone(clone_file);

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);

      facet::file::ptr filefac = clone->get_facet_strict<facet::file>();
      filefac->set_file_repack(true);

      return clone;
    }

  }
}
