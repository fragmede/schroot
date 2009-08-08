/* Copyright © 2005-2008  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-block-device.h"
#include "sbuild-chroot-facet-session-clonable.h"
#ifdef SBUILD_FEATURE_UNION
#include "sbuild-chroot-facet-union.h"
#endif // SBUILD_FEATURE_UNION
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"
#include "sbuild-util.h"

#include <cassert>
#include <cerrno>
#include <cstring>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_block_device::chroot_block_device ():
  chroot_block_device_base()
{
#ifdef SBUILD_FEATURE_UNION
  add_facet(sbuild::chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}

chroot_block_device::~chroot_block_device ()
{
}

chroot_block_device::chroot_block_device (const chroot_block_device& rhs):
  chroot_block_device_base(rhs)
{
}

chroot_block_device::chroot_block_device (const chroot_lvm_snapshot& rhs):
  chroot_block_device_base(rhs)
{
#ifdef SBUILD_FEATURE_UNION
  if (!get_facet<sbuild::chroot_facet_union>())
    add_facet(sbuild::chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}

sbuild::chroot::ptr
chroot_block_device::clone () const
{
  return ptr(new chroot_block_device(*this));
}

sbuild::chroot::ptr
chroot_block_device::clone_session (std::string const& session_id) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_block_device(*this));
  psess->clone_session_setup(session, session_id);

  return session;
}

sbuild::chroot::ptr
chroot_block_device::clone_source () const
{
  ptr clone;

#ifdef SBUILD_FEATURE_UNION
  chroot_facet_union::const_ptr puni(get_facet<chroot_facet_union>());
  assert(puni);

  if (puni->get_union_configured())
    {
      clone = ptr(new chroot_block_device(*this));
      puni->clone_source_setup(clone);
    }
#endif // SBUILD_FEATURE_UNION

  return clone;
}

void
chroot_block_device::setup_env (chroot const& chroot,
				environment&  env) const
{
  chroot_block_device_base::setup_env(chroot, env);
}

sbuild::chroot::session_flags
chroot_block_device::get_session_flags (chroot const& chroot) const
{
  return chroot_block_device_base::get_session_flags(chroot);
}

void
chroot_block_device::get_details (chroot const& chroot,
				  format_detail& detail) const
{
  chroot_block_device_base::get_details(chroot, detail);
}

void
chroot_block_device::get_keyfile (chroot const& chroot,
				  keyfile&      keyfile) const
{
  chroot_block_device_base::get_keyfile(chroot, keyfile);
}

void
chroot_block_device::set_keyfile (chroot&        chroot,
				  keyfile const& keyfile,
				  string_list&   used_keys)
{
  chroot_block_device_base::set_keyfile(chroot, keyfile, used_keys);
}
