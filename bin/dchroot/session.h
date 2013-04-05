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
    session (std::string const&                  service,
             operation                           operation,
             sbuild::session::chroot_list const& chroots);

    /// The destructor.
    virtual ~session ();

    virtual sbuild::string_list
    get_login_directories (sbuild::chroot::ptr&        session_chroot,
                           sbuild::environment const&  env) const;

    virtual void
    get_user_command (sbuild::chroot::ptr&       session_chroot,
                      std::string&               file,
                      sbuild::string_list&       command,
                      sbuild::environment const& env) const;
  };

}

#endif /* DCHROOT_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
