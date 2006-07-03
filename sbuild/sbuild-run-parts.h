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

#ifndef SBUILD_RUN_PARTS_H
#define SBUILD_RUN_PARTS_H

#include <sbuild/sbuild-custom-error.h>
#include <sbuild/sbuild-environment.h>
#include <sbuild/sbuild-types.h>

#include <set>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>

namespace sbuild
{

  /**
   * Run all scripts or programs within a directory.
   */
  class run_parts
  {
  public:
    /// Error codes.
    enum error_code
      {
	CHILD_FORK, ///< Failed to fork child.
	CHILD_WAIT, ///< Wait for child failed.
	EXEC        ///< Failed to execute.
      };

    /// Exception type.
    typedef custom_error<error_code> error;

    /// The constructor.
    run_parts (std::string const& directory,
	       bool               lsb_mode = true,
	       bool               abort_on_error = true,
	       mode_t             umask = 022);

    /// The destructor.
    ~run_parts ();

    /**
     * Get the verbosity level.
     *
     * @returns true if verbose, otherwise false.
     */
    bool
    get_verbose () const;

    /**
     * Set the verbosity level.
     *
     * @param verbose true to be verbose, otherwise false.
     */
    void
    set_verbose (bool verbose);

    /**
     * Get the script execution order.
     *
     * @returns true if executing in reverse, otherwise false.
     */
    bool
    get_reverse () const;

    /**
     * Set the script execution order.
     *
     * @param reverse true to execute in reverse, otherwise false.
     */
    void
    set_reverse (bool reverse);

    int
    run(string_list const& command,
	environment const& env);


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
		 run_parts const&                  rhs)
    {
      if (!rhs.reverse)
	{
	  for (program_set::const_iterator pos = rhs.programs.begin();
	       pos != rhs.programs.end();
	       ++pos)
	    stream << *pos << '\n';
	}
      else
	{
	  for (program_set::const_reverse_iterator pos = rhs.programs.rbegin();
	       pos != rhs.programs.rend();
	       ++pos)
	    stream << *pos << '\n';
	}
      return stream;
    }

  private:
    int
    run_child(std::string const& file,
	      string_list const& command,
	      environment const& env);

    void
    wait_for_child (pid_t pid,
		    int&  child_status);

    bool
    check_filename (std::string const& name);

    typedef std::set<std::string> program_set;

    bool        lsb_mode;
    bool        abort_on_error;
    mode_t      umask;
    bool        verbose;
    bool        reverse;
    //    bool        restricted;
    std::string directory;
    program_set programs;
  };

}

#endif /* SBUILD_RUN_PARTS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
