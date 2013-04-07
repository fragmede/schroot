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

#include <sbuild/chroot/chroot.h>
#include "chroot-facet-source.h"

#include <cassert>

using boost::format;
using std::endl;
using namespace sbuild;

chroot_facet_source::chroot_facet_source ():
  chroot::facet::facet()
{
}

chroot_facet_source::~chroot_facet_source ()
{
}

chroot_facet_source::ptr
chroot_facet_source::create ()
{
  return ptr(new chroot_facet_source());
}

chroot::facet::facet::ptr
chroot_facet_source::clone () const
{
  return ptr(new chroot_facet_source(*this));
}

std::string const&
chroot_facet_source::get_name () const
{
  static const std::string name("source");

  return name;
}

void
chroot_facet_source::setup_env (chroot::chroot const& chroot,
                                environment&          env) const
{
}

chroot::chroot::session_flags
chroot_facet_source::get_session_flags (chroot::chroot const& chroot) const
{
  return chroot::chroot::SESSION_NOFLAGS;
}

void
chroot_facet_source::get_details (chroot::chroot const&  chroot,
                                  format_detail&         detail) const
{
}

void
chroot_facet_source::get_used_keys (string_list& used_keys) const
{
}

void
chroot_facet_source::get_keyfile (chroot::chroot const& chroot,
                                  keyfile&              keyfile) const
{
}

void
chroot_facet_source::set_keyfile (chroot::chroot& chroot,
                                  keyfile const&  keyfile)
{
}
