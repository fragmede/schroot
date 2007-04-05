/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_REGEX_H
#define SBUILD_REGEX_H

#include <istream>
#include <ostream>
#include <string>

#include <boost/regex.hpp>

namespace sbuild
{

  /**
   * POSIX extended regular expression.
   */
  class regex : public boost::regex
  {
  public:
    /// The constructor
    regex ():
      boost::regex()
    {}

    /**
     * The constructor.
     *
     * May throw if the regex is invalid.
     *
     * @param pattern a regex
     */
    regex (std::string const& pattern):
      boost::regex(pattern, boost::regex::extended)
    {}

    /**
     * The constructor.
     *
     * May throw if the regex is invalid.
     *
     * @param pattern a regex
     */
    regex (const char *pattern):
      boost::regex(pattern, boost::regex::extended)
    {}

    ///* The destructor.
    ~regex ()
    {}

    /**
     * Get the regex name from a stream.
     *
     * May throw if the regex is invalid.
     *
     * @param stream the stream to get input from.
     * @param rhs the regex to set.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_istream<charT,traits>&
    operator >> (std::basic_istream<charT,traits>& stream,
		 regex&                            rhs)
    {
      std::string regex;

      if (std::getline(stream, regex))
	{
	  rhs.assign(regex, boost::regex::extended);
	}

      return stream;
    }

    /**
     * Print the regex name to a stream.
     *
     * @param stream the stream to output to.
     * @param rhs the regex to output.
     * @returns the stream.
     */
    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
		 regex const&                rhs)
    {
      return stream << rhs.str();
    }
  };

}

#endif /* SBUILD_REGEX_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
