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
   */
  std::string
  basename (std::string name,
	    char        separator = '/');

  /**
   * Strip the fileame from a pathname.  This is similar to
   * dirname(3).
   *
   * @param name the path to strip of its filename.
   * @param separator the separation delimiting directories.
   * @returns the directory name.
   */
  std::string
  dirname (std::string name,
	   char        separator = '/');

  /**
   * Normalise a pathname.  This strips all trailing separators, and
   * duplicate separators within a path.
   *
   * @param name the path to normalise.
   * @param separator the separation delimiting directories.
   * @returns the normalised name.
   */
  std::string
  normalname (std::string name,
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
  string_list_to_string (string_list const& list,
			 std::string const& separator);

  /**
   * Split a string into a string_list.  The string is split using
   * separator as a delimiter.
   *
   * @param value the string to split.
   * @param separator the delimiting character or characters.
   * @returns a string_list.
   */
  string_list
  split_string (std::string const& value,
		std::string const& separator);

  /**
   * Find a program in the PATH search path.
   *
   * @param program the program to search for.
   * @param path the search path; typically the value of $PATH.
   * @param prefix a directory prefix the add to the search path.
   * This may be left empty to search the root filesystem.
   * @returns the absolute path of the program, or an empty string if
   * the program could not be found.
   */
  std::string
  find_program_in_path (std::string const& program,
			std::string const& path,
			std::string const& prefix);

  /**
   * Create a string vector from a string_list.  The strings in the
   * vector, as well as the vector itself, are allocated with new, and
   * should be freed as a whole with strv_delete.
   *
   * @param str the string_list to use.
   */
  char **
  string_list_to_strv (string_list const& str);

  /**
   * Delete a string vector.  The strings in the vector, as well as
   * the vector itself, must have been previously allocated with new,
   * for example sbuild::environment::get_strv.
   *
   * @param strv the string vector to delete.
   */
  void
  strv_delete (char **strv);

}

#endif /* SBUILD_UTIL_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
