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

#ifndef SCHROOT_MAIN_H
#define SCHROOT_MAIN_H

#include "schroot-options-base.h"

namespace schroot
{

  /**
   * Frontend for schroot.  This class is used to "run" schroot.
   */
  class main
  {
  public:
    /**
     * The constructor.
     *
     * @param options the command-line options to use.
     */
    main (options_base::ptr& options);

    /// The destructor.
    virtual ~main ();

    /**
     * Run the program.
     *
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    int
    run ();

    /**
     * Print version information.
     *
     * @param stream the stream to output to.
     */
    virtual void
    action_version (std::ostream& stream);

    /**
     * List chroots.
     */
    virtual void
    action_list ();

    /**
     * Print detailed information about chroots.
     */
    virtual void
    action_info ();

    /**
     * Dump configuration file for chroots.
     */
    virtual void
    action_config ();

    /**
     * Print location of chroots.
     */
    virtual void
    action_location ();

  protected:
    /**
     * Get a list of chroots based on the specified options (--all, --chroot).
     *
     * @returns a list of chroots.
     */
    virtual sbuild::string_list
    get_chroot_options ();

    /**
     * Check compatibility.  Does nothing, but derived classes may use
     * it as they see fit.
     */
    virtual void
    compat_check ();

    /**
     * Load configuration.
     */
    virtual void
    load_config();

    /**
     * Create a session.  This sets the session member.
     */
    virtual void
    create_session(sbuild::session::operation sess_op);

  protected:
    /// The name of the program.
    std::string                program_name;
    /// The program options.
    options_base::ptr          options;
    /// The chroot configuration.
    sbuild::chroot_config::ptr config;
    /// The chroots to use.
    sbuild::string_list        chroots;
    /// The session.
    sbuild::session::ptr       session;
  };

}

#endif /* SCHROOT_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
