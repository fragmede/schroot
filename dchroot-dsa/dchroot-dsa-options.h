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

#ifndef DCHROOT_DSA_OPTIONS_H
#define DCHROOT_DSA_OPTIONS_H

#include <schroot/schroot-options-base.h>

namespace dchroot_dsa
{

  /**
   * dchroot-dsa command-line options.
   */
  class options : public schroot::options_base
  {
  public:

    /**
     * The constructor.
     *
     * @param argc the number of arguments.
     * @param argv the list of arguments.
     */
    options ();

    /// The destructor.
    virtual ~options ();

  protected:
    virtual void
    add_options ();

    virtual void
    check_options ();
  };

}

#endif /* DCHROOT_DSA_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */

