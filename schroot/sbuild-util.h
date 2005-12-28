/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_UTIL_H
#define SBUILD_UTIL_H

#include <string>

#include "sbuild-types.h"

namespace sbuild
{

  /**
   * Strip the directory path from a filename.  This is similar to
   * basename(3).
   *
   * @param name the filename to strip of its path.
   * @param separator the separation delimiting directories.
   * @returns the base name.
   *
   * @todo Regression test with basename(3) examples.
   */
  std::string
  basename(std::string name,
	   char        separator = '/');

  /**
   * Strip the fileame from a pathname.  This is similar to
   * dirname(3).
   *
   * @param name the path to strip of its filename.
   * @param separator the separation delimiting directories.
   * @returns the directory name.
   *
   * @todo Regression test with dirname(3) examples.
   */
  std::string
  dirname(std::string name,
	  char        separator = '/');

  /**
   * Convert a string_list into a string.  The strings are
   * concatenated using separator as a delimiter.
   *
   * @param list the list to concatenate.
   * @param separator the delimiting character.
   * @returns a string.
   */
  std::string
  string_list_to_string(string_list const& list,
			std::string const& separator);

  /**
   * Split a string into a string_list.  The string is split using
   * separator as a delimiter.
   *
   * @param value the string to split.
   * @param separator the delimiting character.
   * @returns a string_list.
   */
  string_list
  split_string(const std::string& value,
	       char               separator);

  /**
   * Find a program in the PATH search path.
   *
   * @param program the program to search for.
   * @returns the absolute path of the program, or an empty string if
   * the program could not be found.
   *
   * @todo Add the path as an additional argument, rather than
   * assuming $PATH.  This will allow the PATH to be set within the
   * chroot to be used.
   */
  std::string
  find_program_in_path(const std::string& program);

}

#endif /* SBUILD_UTIL_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
