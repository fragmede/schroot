/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef DCHROOT_DSA_SESSION_H
#define DCHROOT_DSA_SESSION_H

#include <dchroot/dchroot-session-base.h>

namespace dchroot_dsa
{

  /**
   * Session handler for dchroot-dsa sessions.
   *
   * This class provides the session handling for dchroot-dsa
   * compatibility.  It overrides the normal authentication checks to
   * allow all users to access the service, but enforce dchroot-dsa
   * user access controls when present, and it specialises the session
   * behaviour to be compatible with the chdir and command execution
   * behaviour of dchroot-dsa.
   */
  class session : public dchroot::session_base
  {
  public:
    /**
     * The constructor.
     *
     * @param service the PAM service name.
     * @param operation the session operation to perform.
     * @param chroots the chroots to act upon.
     * @param compat true to enable full dchroot compatibility, or
     * false to enable schroot compatiblity (permissions checks).
     */
    session (std::string const&         service,
	     operation                  operation,
		  sbuild::session::chroot_list const& chroots,
	     bool                       compat);

    /// The destructor.
    virtual ~session ();

    virtual sbuild::auth::status
    get_chroot_auth_status (sbuild::auth::status status,
			    sbuild::chroot::ptr const& chroot) const;

    virtual sbuild::string_list
    get_login_directories (sbuild::chroot::ptr&       session_chroot,
			   sbuild::environment const& env) const;

    virtual void
    get_user_command (sbuild::chroot::ptr&       session_chroot,
		      std::string&               file,
		      sbuild::string_list&       command,
		      sbuild::environment const& env) const;
  };

}

#endif /* DCHROOT_DSA_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
