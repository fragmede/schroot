/* Copyright © 2005-2007  Roger Leigh <rleigh@codelibre.net>
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

#ifndef SCHROOT_MAIN_H
#define SCHROOT_MAIN_H

#include <schroot/schroot-main-base.h>
#include <schroot/schroot-options-base.h>

/**
 * schroot program components.
 */
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

  protected:
    virtual void
    create_session(sbuild::session::operation sess_op);

    virtual void
    add_session_auth ();
  };

}

#endif /* SCHROOT_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
