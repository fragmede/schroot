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

#ifndef DCHROOT_SESSION_BASE_H
#define DCHROOT_SESSION_BASE_H

#include <sbuild/sbuild-session.h>

namespace dchroot
{

  /**
   * Basic session handler for dchroot sessions.
   *
   * This class provides common session functionality for dchroot and
   * dchroot-dsa, such as providing a schroot compatibility mode.  It
   * also prevents user switching when running sessions, which is
   * forbidden.
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
    get_command_directories (sbuild::chroot::ptr&       session_chroot,
			     sbuild::environment const& env) const;

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
