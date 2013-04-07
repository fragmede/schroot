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

#ifndef SBUILD_CHROOT_BLOCK_DEVICE_H
#define SBUILD_CHROOT_BLOCK_DEVICE_H

#include <sbuild/config.h>
#include <sbuild/chroot/block-device-base.h>
#include <sbuild/chroot/lvm-snapshot.h>

namespace sbuild
{
  namespace chroot
  {

    /**
     * A chroot stored on an unmounted block device.
     *
     * The device will be mounted on demand.
     */
    class block_device : public block_device_base
    {
    public:
      /// Exception type.
      typedef chroot::error error;

    protected:
      /// The constructor.
      block_device ();

      /// The copy constructor.
      block_device (const block_device& rhs);

#ifdef SBUILD_FEATURE_LVMSNAP
      /// The copy constructor.
      block_device (const lvm_snapshot& rhs);
#endif

      friend class chroot;
#ifdef SBUILD_FEATURE_LVMSNAP
      friend class lvm_snapshot;
#endif

    public:
      /// The destructor.
      virtual ~block_device ();

      virtual chroot::ptr
      clone () const;

      virtual chroot::ptr
      clone_session (std::string const& session_id,
                     std::string const& alias,
                     std::string const& user,
                     bool               root) const;

      virtual chroot::ptr
      clone_source () const;

      std::string const&
      get_chroot_type () const;

      virtual void
      setup_env (chroot const& chroot,
                 environment& env) const;

      virtual session_flags
      get_session_flags (chroot const& chroot) const;

    protected:
      virtual void
      setup_lock (chroot::setup_type type,
                  bool               lock,
                  int                status);


      virtual void
      get_details (chroot const&  chroot,
                   format_detail& detail) const;

      virtual void
      get_used_keys (string_list& used_keys) const;

      virtual void
      get_keyfile (chroot const& chroot,
                   keyfile&      keyfile) const;

      virtual void
      set_keyfile (chroot&        chroot,
                   keyfile const& keyfile);
    };

  }
}

#endif /* SBUILD_CHROOT_BLOCK_DEVICE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
