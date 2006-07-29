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

#include <sbuild/sbuild-environment.h>
#include <sbuild/sbuild-types.h>

#include <string>

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
   * Check if a pathname is absolute.
   *
   * @param name the path to check.
   * @returns true if the name is absolute or false if it is not, or
   * if name is empty.
   */
  bool
  is_absname (std::string const& name);

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
   *
   * @todo Provide an alternative that splits the string in place
   * using an iterator interface.
   */
  string_list
  split_string (std::string const& value,
		std::string const& separator);

  /**
   * Widen a string.  The narrow string is converted into a wide
   * string.  Note that any conversion error will cause the string to
   * be clipped at the point of error.
   *
   * @param str the string to widen.
   * @param locale the locale to use for the conversion.
   * @returns a wide string.
   */
  std::wstring
  widen_string (std::string const& str,
		std::locale        locale);

  /**
   * Narrow a string.  The wide string is converted into a narrow
   * string.  Note that any conversion error will cause the string to
   * be clipped at the point of error.
   *
   * @param str the string to narrow.
   * @param locale the locale to use for the conversion.
   * @returns a narrow string.
   */
  std::string
  narrow_string (std::wstring const& str,
		 std::locale         locale);

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

  /**
   * execve wrapper.  Run the command specified by file (an absolute
   * pathname), using command and env as the argv and environment,
   * respectively.
   *
   * @param file the program to execute.
   * @param command the arguments to pass to the executable.
   * @param env the environment.
   * @returns the return value of the execve system call on failure.
   */
  int
  exec (std::string const& file,
	string_list const& command,
	environment const& env);
}

#endif /* SBUILD_UTIL_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
