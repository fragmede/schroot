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
       *
       * @param stream the stream to output to.
       * @param rhs the null to output.
       * @returns the stream.
       */
      template <class charT, class traits>
      friend
      std::basic_ostream<charT,traits>&
      operator << (std::basic_ostream<charT,traits>& stream,
                   null const&                       rhs)
      {
        return stream << null_output();
      }

    private:
      /**
       * Get a string for output.
       *
       * @returns the word "unknown" (translated).
       */
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
