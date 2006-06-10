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

#ifndef SCHROOT_LISTMOUNTS_OPTIONS_H
#define SCHROOT_LISTMOUNTS_OPTIONS_H

#include <string>

namespace schroot_listmounts
{

  /**
   * schroot-listmounts command-line options.
   */
  class options {
  public:
    /**
     * The constructor.
     *
     * @param argc the number of arguments.
     * @param argv the list of arguments.
     */
    options (int   argc,
	     char *argv[]);

    /// The destructor.
    virtual ~options ();

    /// The mountpoint to check.
    std::string mountpoint;
    /// Display version information.
    bool        version;
  };

}

#endif /* SCHROOT_LISTMOUNTS_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
