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

#ifndef SCHROOT_SBUILD_SESSION_H
#define SCHROOT_SBUILD_SESSION_H

#include <sbuild/sbuild-session.h>

namespace schroot_sbuild
{

  /**
   * Session handler for schroot-sbuild sessions.
   */
  class session : public sbuild::session
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

    virtual sbuild::auth::status
    get_chroot_auth_status (sbuild::auth::status       status,
			    sbuild::chroot::ptr const& chroot) const;
  };

}

#endif /* SCHROOT_SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
