/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
 *
 * schroot is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * schroot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 *
 *********************************************************************/

#ifndef DCHROOT_SESSION_BASE_H
#define DCHROOT_SESSION_BASE_H

#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "sbuild-session.h"

namespace dchroot
{

  /**
   * Session handler for dchroot sessions.
   *
   * This class provides the session handling for dchroot
   * compatibility.  It derives from session, overriding the
   * authentication checks to allow all users to access the service,
   * and does not permit user switching.
   */
  class session_base : public sbuild::session
  {
  public:
    /**
     * The constructor.
     *
     * @param service the PAM service name.
     * @param config a shared_ptr to the chroot configuration.
     * @param operation the session operation to perform.
     * @param chroots the chroots to act upon.
     * @param compat true to enable full dchroot compatibility, or
     * false to enable schroot compatiblity (permissions checks).
     */
    session_base (std::string const&         service,
		  config_ptr&                config,
		  operation                  operation,
		  sbuild::string_list const& chroots,
		  bool                       compat);

    /// The destructor.
    virtual ~session_base ();

    /**
     * Get the dchroot compatibility state.
     *
     * @returns the state.
     */
    bool
    get_compat () const;

    /**
     * Set the dchroot compatibility state.
     *
     * @param state the dchroot compatibility state.
     */
    void
    set_compat (bool state);

  protected:
    virtual void
    run_impl ();

    virtual sbuild::string_list
    get_command_directories () const;

  private:
    /// dchroot compatibility enabled?
    bool compat;
  };

}

#endif /* DCHROOT_SESSION_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
