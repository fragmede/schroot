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

#ifndef SBUILD_CHROOT_LOOPBACK_H
#define SBUILD_CHROOT_LOOPBACK_H

#include <sbuild/config.h>
#include <sbuild/chroot/chroot.h>

namespace sbuild
{
  namespace chroot
  {
    /**
     * A chroot stored in a file for loopback mounting.
     *
     * The file will be mounted on demand.
     */
    class loopback : public chroot
    {
    public:
      /// Exception type.
      typedef chroot::error error;

    protected:
      /// The constructor.
      loopback ();

      /// The copy constructor.
      loopback (const loopback& rhs);

      friend class chroot;

    public:
      /// The destructor.
      virtual ~loopback ();

      virtual chroot::ptr
      clone () const;

      virtual chroot::ptr
      clone_session (std::string const& session_id,
                     std::string const& alias,
                     std::string const& user,
                     bool               root) const;

      virtual chroot::ptr
      clone_source () const;

      /**
       * Get the filename containing the chroot.
       *
       * @returns the filename.
       */
      std::string const&
      get_filename () const;

      /**
       * Set the filename containing the chroot.
       *
       * @param filename the filename.
       */
      void
      set_filename (std::string const& filename);

      std::string const&
      get_chroot_type () const;

      virtual std::string
      get_path () const;

      virtual void
      setup_env (chroot const& chroot,
                 environment&  env) const;

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

    private:
      /// The file to use.
      std::string filename;
    };

  }
}

#endif /* SBUILD_CHROOT_LOOPBACK_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
