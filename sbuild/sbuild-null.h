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

#ifndef SBUILD_NULL_H
#define SBUILD_NULL_H

#include <map>
#include <stdexcept>
#include <string>

namespace sbuild
{

  /**
   * Null.  This class does nothing.  It is used to represent an
   * absence of context or detail.
   */
  class null
    {
    public:
      /**
       * Null output to an ostream.
       */
      template <class charT, class traits>
      friend
      std::basic_ostream<charT,traits>&
      operator << (std::basic_ostream<charT,traits>& stream,
		   null const&                       n)
      {
	return stream << null_output();
      }

    private:
      static const char *
      null_output ();
  };

}

#endif /* SBUILD_NULL_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
