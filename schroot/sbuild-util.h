/* sbuild-util - sbuild utility functions
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_UTIL
#define SBUILD_UTIL

#include <string>

namespace Sbuild
{
  std::string basename(std::string name,
		       char        separator = '/');

  std::string dirname(std::string name,
		      char        separator = '/');
}

#endif /* SBUILD_UTIL */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
