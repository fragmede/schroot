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

#include <sbuild/chroot/block-device.h>
#include <sbuild/chroot/facet/session-clonable.h>
#include <sbuild/chroot/facet/source-clonable.h>
#ifdef SBUILD_FEATURE_UNION
#include <sbuild/chroot/facet/fsunion.h>
#endif // SBUILD_FEATURE_UNION
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

    block_device::block_device ():
      block_device_base()
    {
#ifdef SBUILD_FEATURE_UNION
      add_facet(facet::fsunion::create());
#endif // SBUILD_FEATURE_UNION
    }

    block_device::~block_device ()
    {
    }

    block_device::block_device (const block_device& rhs):
      block_device_base(rhs)
    {
    }

#ifdef SBUILD_FEATURE_LVMSNAP
    block_device::block_device (const lvm_snapshot& rhs):
      block_device_base(rhs)
    {
#ifdef SBUILD_FEATURE_UNION
      if (!get_facet<facet::fsunion>())
        add_facet(facet::fsunion::create());
#endif // SBUILD_FEATURE_UNION
    }
#endif // SBUILD_FEATURE_LVMSNAP

    chroot::chroot::ptr
    block_device::clone () const
    {
      return ptr(new block_device(*this));
    }

    chroot::chroot::ptr
    block_device::clone_session (std::string const& session_id,
                                 std::string const& alias,
                                 std::string const& user,
                                 bool               root) const
    {
      facet::session_clonable::const_ptr psess
        (get_facet<facet::session_clonable>());
      assert(psess);

      ptr session(new block_device(*this));
      psess->clone_session_setup(*this, session, session_id, alias, user, root);

      return session;
    }

    chroot::chroot::ptr
    block_device::clone_source () const
    {
      ptr clone(new block_device(*this));

      facet::source_clonable::const_ptr psrc
        (get_facet<facet::source_clonable>());
      assert(psrc);

      psrc->clone_source_setup(*this, clone);

      return clone;
    }

    void
    block_device::setup_env (chroot const& chroot,
                             environment&  env) const
    {
      block_device_base::setup_env(chroot, env);
    }

    void
    block_device::setup_lock (chroot::setup_type type,
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
              /* We don't lock the device if fsunion is configured. */
              const chroot *base = dynamic_cast<const chroot *>(this);
              assert(base);
              facet::fsunion::const_ptr puni
                (base->get_facet<facet::fsunion>());
#endif // SBUILD_FEATURE_UNION
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
    block_device::get_chroot_type () const
    {
      static const std::string type("block-device");

      return type;
    }

    chroot::chroot::session_flags
    block_device::get_session_flags (chroot const& chroot) const
    {
      return block_device_base::get_session_flags(chroot);
    }

    void
    block_device::get_details (chroot const& chroot,
                               format_detail& detail) const
    {
      block_device_base::get_details(chroot, detail);
    }

    void
    block_device::get_used_keys (string_list& used_keys) const
    {
      block_device_base::get_used_keys(used_keys);
    }

    void
    block_device::get_keyfile (chroot const& chroot,
                               keyfile&      keyfile) const
    {
      block_device_base::get_keyfile(chroot, keyfile);
    }

    void
    block_device::set_keyfile (chroot&        chroot,
                               keyfile const& keyfile)
    {
      block_device_base::set_keyfile(chroot, keyfile);
    }

  }
}
