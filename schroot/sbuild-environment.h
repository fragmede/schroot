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

#include "sbuild-log.h"
#include "sbuild-parse-value.h"

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
     * @param environment the environment to set.
     */
    environment(char **environment);

    /// The destructor.
    ~environment();

    /**
     * Add environment variables.  Any existing variables sharing the
     * name of a new value will be replaced.
     *
     * @param environment the environment variables to add.  This is a
     * null-terminated array of pointers to char.
     */
    void
    add (char **environment);

    /**
     * Add environment variables.  Any existing variables sharing the
     * name of a new value will be replaced.
     *
     * @param environment the environment variables to add.
     */
    void
    add (environment const& environment);

    /**
     * Add environment variable.  Any existing variable sharing the
     * name will be replaced.
     *
     * @param value the environment variable to add.
     */
    void
    add (value_type const& value);

    /**
     * Add environment variable.  Any existing variable sharing the
     * name will be replaced.
     *
     * @param name the environment variable name
     * @param value the environment variable value to add.
     */
    void
    add(std::string const& name,
	std::string const& value)
    {
      add(std::make_pair(name, value));
    }

    /**
     * Add environment variable.  Any existing variable sharing the
     * name will be replaced.
     *
     * @param name the environment variable name
     * @param value the environment variable value to add.
     */
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

    /**
     * Add environment variable.  Any existing variable sharing the
     * name will be replaced.
     *
     * @param value the environment variable to add.  This is a string
     * in the form key=value.
     */
    void
    add (std::string const& value);

    /**
     * Remove environment variables.  Any variables sharing the names
     * of a specified value will be removed.
     *
     * @param environment the environment variables to remove.  This
     * is a null-terminated array of pointers to char.
     */
    void
    remove (char **environment);

    /**
     * Remove environment variables.  Any variables sharing the names
     * of a specified value will be removed.
     *
     * @param environment the environment variables to remove.
     */
    void
    remove (environment const& environment);

    /**
     * Remove environment variable.  Any variable sharing the name
     * of the specified value will be removed.
     *
     * @param value the environment variable to remove.
     */
    void
    remove (std::string const& value);

    /**
     * Remove environment variable.  Any variable sharing the name
     * of the specified value will be removed.
     *
     * @param value the environment variable to remove.
     */
    void
    remove (value_type const& value);

    /**
     * Get the value of an environment variable.
     *
     * @param name the name of the environment variable.
     * @param value the variable to store the value in.
     * @returns true on success, false if the variable does not exist,
     * or there is a parse error.
     */
    template <typename T>
    bool
    get (std::string const& name,
	 T&                 value)
    {
      log_debug(DEBUG_INFO) << "Getting environment variable=" << name
			    << std::endl;
      iterator pos = find(name);
      if (pos != end())
	return parse_value(pos->second, value);
      log_debug(DEBUG_NOTICE) << "name not found: " << name << std::endl;
      return false;
    }

    /**
     * Get the evironment variables as a string vector.  This form is
     * suitable for use as an envp argument with execve, for example.
     *
     * @returns a newly-allocated string vector.  This is allocated
     * with new, and should be freed with strv_delete().
     */
    char **
    get_strv() const;

    /**
     * Add variables to the environment.
     *
     * @param rhs the values to add.
     * @returns the modified environment.
     */
    template <typename T>
    environment&
    operator += (T& rhs)
    {
      add(rhs);
      return *this;
    }

    /**
     * Remove variables from the environment.
     *
     * @param rhs the values to remove.
     * @returns the modified environment.
     */
    template <typename T>
    environment&
    operator -= (T& rhs)
    {
      remove(rhs);
      return *this;
    }

    /**
     * Add variables to the environment.
     *
     * @param lhs the environment to add to.
     * @param rhs the values to add.
     * @returns the new environment.
     */
    template <typename T>
    friend environment
    operator + (environment const& lhs,
		T const&           rhs)
    {
      environment ret(lhs);
      ret += rhs;
      return ret;
    }

    /**
     * Remove variables from the environment.
     *
     * @param lhs the environment to remove from.
     * @param rhs the values to remove.
     * @returns the new environment.
     */
    template <typename T>
    friend environment
    operator - (environment const& lhs,
		T const&           rhs)
    {
      environment ret(lhs);
      ret -= rhs;
      return ret;
    }

    /**
     * Output the environment to an ostream.
     *
     * @param stream the stream to output to.
     * @param rhs the environment to output.
     * @returns the stream.
     */
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
