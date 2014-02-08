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

#ifndef DCHROOT_SESSION_H
#define DCHROOT_SESSION_H

#include <dchroot-common/session.h>

namespace bin
{
  namespace dchroot
  {

    /**
     * Session handler for dchroot sessions.
     *
     * This class provides the session handling for dchroot
     * compatibility.  It specialises the session behaviour to be
     * compatible with the chdir and command execution behaviour of
     * dchroot.
     */
    class session : public dchroot_common::session
    {
    public:
      /**
       * The constructor.
       *
       * @param service the PAM service name.
       * @param operation the session operation to perform.
       * @param chroots the chroots to act upon.
       */
      session (const std::string&                   service,
               operation                            operation,
               const schroot::session::chroot_list& chroots);

      /// The destructor.
      virtual ~session ();

      virtual schroot::string_list
      get_login_directories (schroot::chroot::chroot::ptr& session_chroot,
                             const schroot::environment&   env) const;

      virtual void
      get_user_command (schroot::chroot::chroot::ptr& session_chroot,
                        std::string&                  file,
                        schroot::string_list&         command,
                        const schroot::environment&   env) const;
    };

  }
}

#endif /* DCHROOT_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
