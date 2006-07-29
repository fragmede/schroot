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

#include <schroot/schroot-main-base.h>
#include <schroot/schroot-options-base.h>

namespace schroot
{

  /**
   * Frontend for schroot.  This class is used to "run" schroot.
   */
  class main : public main_base
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
     * List chroots.
     */
    virtual void
    action_list ();

    /**
     * Dump configuration file for chroots.
     */
    virtual void
    action_config ();

  protected:
    virtual void
    create_session(sbuild::session::operation sess_op);
  };

}

#endif /* SCHROOT_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
