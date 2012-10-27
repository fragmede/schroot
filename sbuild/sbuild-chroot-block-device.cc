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
#include "sbuild-chroot-facet-source-clonable.h"
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
  add_facet(chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}

chroot_block_device::~chroot_block_device ()
{
}

chroot_block_device::chroot_block_device (const chroot_block_device& rhs):
  chroot_block_device_base(rhs)
{
}

#ifdef SBUILD_FEATURE_LVMSNAP
chroot_block_device::chroot_block_device (const chroot_lvm_snapshot& rhs):
  chroot_block_device_base(rhs)
{
#ifdef SBUILD_FEATURE_UNION
  if (!get_facet<chroot_facet_union>())
    add_facet(chroot_facet_union::create());
#endif // SBUILD_FEATURE_UNION
}
#endif // SBUILD_FEATURE_LVMSNAP

sbuild::chroot::ptr
chroot_block_device::clone () const
{
  return ptr(new chroot_block_device(*this));
}

sbuild::chroot::ptr
chroot_block_device::clone_session (std::string const& session_id,
				    std::string const& alias,
				    std::string const& user,
				    bool               root) const
{
  chroot_facet_session_clonable::const_ptr psess
    (get_facet<chroot_facet_session_clonable>());
  assert(psess);

  ptr session(new chroot_block_device(*this));
  psess->clone_session_setup(*this, session, session_id, alias, user, root);

  return session;
}

sbuild::chroot::ptr
chroot_block_device::clone_source () const
{
  ptr clone(new chroot_block_device(*this));

  chroot_facet_source_clonable::const_ptr psrc
    (get_facet<chroot_facet_source_clonable>());
  assert(psrc);

  psrc->clone_source_setup(*this, clone);

  return clone;
}

void
chroot_block_device::setup_env (chroot const& chroot,
				environment&  env) const
{
  chroot_block_device_base::setup_env(chroot, env);
}

void
chroot_block_device::setup_lock (chroot::setup_type type,
				 bool               lock,
				 int                status)
{
  /* Lock is preserved through the entire session. */
  if ((type == SETUP_START && lock == false) ||
      (type == SETUP_STOP && lock == true))
    return;

  try
    {
      if (!stat
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
	  (this->get_device()).is_character()
#else
	  (this->get_device()).is_block()
#endif
	  )
	{
	  throw error(get_device(), DEVICE_NOTBLOCK);
	}
      else
	{
#ifdef SBUILD_FEATURE_UNION
	  /* We don't lock the device if union is configured. */
	  const chroot *base = dynamic_cast<const chroot *>(this);
	  assert(base);
	  chroot_facet_union::const_ptr puni
	    (base->get_facet<chroot_facet_union>());
	  if (!puni || !puni->get_union_configured())
#endif
	    {
	      device_lock dlock(this->device);
	      if (lock)
		{
		  try
		    {
		      dlock.set_lock(lock::LOCK_EXCLUSIVE, 15);
		    }
		  catch (lock::error const& e)
		    {
		      throw error(get_device(), DEVICE_LOCK, e);
		    }
		}
	      else
		{
		  try
		    {
		      dlock.unset_lock();
		    }
		  catch (lock::error const& e)
		    {
		      throw error(get_device(), DEVICE_UNLOCK, e);
		    }
		}
	    }
	}
    }
  catch (sbuild::stat::error const& e) // Failed to stat
    {
      // Don't throw if stopping a session and the device stat
      // failed.  This is because the setup scripts shouldn't fail
      // to be run if the block device no longer exists, which
      // would prevent the session from being ended.
      if (type != SETUP_STOP)
	throw;
    }

  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

std::string const&
chroot_block_device::get_chroot_type () const
{
  static const std::string type("block-device");

  return type;
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
