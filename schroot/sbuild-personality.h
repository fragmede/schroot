/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_PERSONALITY_H
#define SBUILD_PERSONALITY_H

#include "sbuild-config.h"

#include <map>
#include <ostream>
#include <string>

namespace sbuild
{

  /**
   * Chroot personality.  A chroot may have a personality (also knows
   * as a process execution domain) which is used to run non-native
   * binaries.  For example, running 32-bit Linux binaries on a 64-bit
   * Linux system, or an SVR4 binary on a 32-bit Linux system.  This
   * is currently a Linux only feature; it does nothing on non-Linux
   * systems.  This is a wrapper around the personality(2) system
   * call.
   */
  class personality
  {
  public:
    /// Personality type.
    typedef unsigned long type;

    /// Exception type.
    typedef runtime_error_custom<personality> error;

    /// The constructor.
    personality ();

    /**
     * The constructor.
     */
    personality (type persona);

    /**
     * The constructor.
     */
    personality (std::string const& persona);

    ///* The destructor.
    ~personality ();

    /**
     * Get the name of the personality.
     *
     * @returns the personality name.
     */
    std::string const& get_name () const;

    /**
     * Set personality.  This sets the personality (if valid) using
     * the personality(2) system call.  If setting the personality
     * fails, an error is thown.
     */
    void
    set () const;

    /**
     * Print a list of the available personalities.
     *
     * @param stream the stream to output to.
     */
    static void
    print_personalities (std::ostream& stream);

    /**
     * Print the personality name to a stream.
     *
     * @param stream the stream to output to.
     * @param rhs the personality to output.
     * @returns the stream.
     */
    friend std::ostream&
    operator << (std::ostream&      stream,
		 personality const& rhs)
    {
      return stream << find_personality(rhs.persona);
    }

  private:
    /**
     * Find a personality by name.
     *
     * @param persona the personality to find.
     * @returns the personality type; this is -1 if the personality
     * was undefined, or -2 if the personality was unknown (not
     * found).
     */
    static type
    find_personality (std::string const& persona);

    /**
     * Find a personality by number.
     *
     * @param persona the personality to find.
     * @returns the personality name, "undefined" if the personality was
     * not defined, or "unknown" if the personality was not found.
     */
    static std::string const&
    find_personality (type persona);

    /// The personality type.
    type persona;

    /// Mapping between personality name and type.
    static std::map<std::string,type> personalities;
  };

}

#endif /* SBUILD_PERSONALITY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
