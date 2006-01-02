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

#ifndef SBUILD_ENVIRONMENT_H
#define SBUILD_ENVIRONMENT_H

#include <map>
#include <string>
#include <sstream>

namespace sbuild
{

  /**
   * Container of environment variables.
   */
  class environment : public std::map<std::string, std::string>
  {
  public:
    using std::map<std::string, std::string>::value_type;

    /// The constructor.
    environment();

    /**
     * The constructor.
     *
     * @environment the environment to set.
     */
    environment(char **environment);

    /// The destructor.
    ~environment();

    void
    add (char **environment);

    void
    add (environment const& environment);

    void
    add (value_type const& value);

    template<typename T>
    void
    add(std::string const& name,
	T const&           value)
    {
      std::ostringstream varstring;
      varstring.imbue(std::locale("C"));
      varstring << value;
      add(std::make_pair(name, varstring.str()));
    }

    void
    add (std::string const& value);

    void
    remove (char **environment);

    void
    remove (environment const& environment);

    void
    remove (std::string const& value);

    void
    remove (value_type const& value);

    char **
    get_strv() const;

    template <typename T>
    environment&
    operator += (T const& rhs)
    {
      add(rhs);
      return *this;
    }

    template <typename T>
    environment&
    operator -= (T const& rhs)
    {
      remove(rhs);
      return *this;
    }


    template <typename T>
    friend environment
    operator + (environment const& lhs,
		T const&           rhs)
    {
      environment ret(lhs);
      lhs += rhs;
      return ret;
    }

    template <typename T>
    friend environment
    operator - (environment const& lhs,
		T const&           rhs)
    {
      environment ret(lhs);
      lhs -= rhs;
      return ret;
    }

    template <class charT, class traits>
    friend
    std::basic_ostream<charT,traits>&
    operator << (std::basic_ostream<charT,traits>& stream,
		 environment const& rhs)
    {
      for (environment::const_iterator pos = rhs.begin();
	   pos != rhs.end();
	   ++pos)
	{
	  stream << pos->first << '=' << pos->second << '\n';
	}

      return stream;
    }
  };

}

#endif /* SBUILD_ENVIRONMENT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
