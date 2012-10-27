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

#ifndef DCHROOT_MAIN_H
#define DCHROOT_MAIN_H

#include <dchroot/dchroot-main-base.h>

/**
 * dchroot program components.
 */
namespace dchroot
{

  /**
   * Frontend for dchroot.  This class is used to "run" dchroot.
   */
  class main : public dchroot::main_base
  {
  public:
    /**
     * The constructor.
     *
     * @param options the command-line options to use.
     */
    main (schroot::options_base::ptr& options);

    /// The destructor.
    virtual ~main ();

  protected:
    virtual void
    create_session (sbuild::session::operation sess_op);
  };

}

#endif /* DCHROOT_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
