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

#ifndef SCHROOT_BASE_MAIN_H
#define SCHROOT_BASE_MAIN_H

#include <schroot/schroot-base-options.h>

#include <string>

namespace schroot_base
{

  /**
   * Frontend base for schroot programs.  This class is used to "run"
   * schroot programs.  It contains functionality common to all
   * programs, such as help and version output.
   */
  class main
  {
  public:
    /**
     * The constructor.
     *
     * @param program_name the program name.
     * @param program_usage the program usage message.
     * @param program_options the command-line options to use.
     */
    main (std::string const&  program_name,
	  std::string const&  program_usage,
	  options::ptr const& program_options);

    /// The destructor.
    virtual ~main ();

    /**
     * Run the program.
     *
     * @param argc the number of arguments
     * @param argv argument vector
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    int
    run (int   argc,
	 char *argv[]);

    /**
     * Print help information.
     *
     * @param stream the stream to output to.
     */
    virtual void
    action_help (std::ostream& stream);

    /**
     * Print version information.
     *
     * @param stream the stream to output to.
     */
    virtual void
    action_version (std::ostream& stream);

  protected:
    /**
     * Run the program.  This is the program-specific run method which
     * must be implemented in a derived class.
     *
     * @returns 0 on success, 1 on failure or the exit status of the
     * chroot command.
     */
    virtual int
    run_impl () = 0;

    /// The name of the program.
    std::string  program_name;
    /// The usage text of the program.
    std::string  program_usage;
    /// The program options.
    options::ptr program_options;
  };

}

#endif /* SCHROOT_BASE_MAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
