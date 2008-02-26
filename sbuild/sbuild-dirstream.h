/* Copyright © 2003,2006-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_DIRSTREAM_H
#define SBUILD_DIRSTREAM_H

#include <sbuild/sbuild-custom-error.h>

#include <cstring>
#include <iostream>
#include <deque>
#include <string>

#include <sys/types.h>
#include <dirent.h>

namespace sbuild
{

  /**
   * An entry in a dirstream.  It is a wrapper around the dirent
   * structure declared in dirent.h.  Unlike a dirent pointer returned
   * by readdir(3), a direntry does not become invalid when the
   * dirstream it was extracted from is destroyed.
   */
  class direntry
  {
  public:
    /// The constructor.
    direntry()
    { std::memset(&this->data, 0, sizeof(struct dirent)); }

    /**
     * The constructor.
     *
     * @param entry the dirent to initialise the class with.
     */
    direntry(const struct dirent *entry)
    { std::memcpy(&this->data, entry, sizeof(struct dirent)); }

    /**
     * The copy constructor.
     *
     * @param orig the class to copy.
     */
    direntry(direntry const& orig)
    { memcpy(&this->data, &orig.data, sizeof(struct dirent)); }

    /// The destructor.
    virtual ~direntry()
    {}

    /**
     * Get the dirent inode number (d_ino).
     *
     * @returns the inode number.
     */
    long inode() const
    { return this->data.d_ino; }

    /**
     * Get the file type (d_type).
     *
     * @returns the file type.
     */
    unsigned char type() const
    { return this->data.d_type; }

    /**
     * Get the file name (d_name).
     *
     * @returns a reference to a string containing the name.
     */
    std::string name() const
    { return this->data.d_name; }

    /**
     * Get the dirent.
     *
     * @returns a reference to the underlying dirent.
     */
    struct dirent const& dirent()
    { return this->data; }

  private:
    /// The underlying dirent the class is wrapping.
    struct dirent data;
  }; // class direntry

  /**
   * Access directories.  This is a wrapper around the opendir(3),
   * readdir(3) and closedir(3) functions, which are used to read a
   * stream of "dirents" through multiple readdir() calls.
   *
   * dirstream calls opendir() and closedir() automatically, and
   * represents each dirent as a dirstream::direntry.  Like reading
   * from and istream by pulling data out with the >> "extraction
   * operator", direntries are also extracted from the dirstream with
   * the >> operator.
   */
  class dirstream
    {
    public:
      /// Error codes.
      enum error_code
	{
	  DIR_OPEN, ///< Failed to open directory.
	  DIR_READ  ///< Failed to read directory.
	};

    /// Exception type.
    typedef custom_error<error_code> error;

      /**
       * The constructor.
       *
       * @param dir the directory to read.
       */
      dirstream(std::string const& dir);

      /// The destructor.
      virtual ~dirstream();

      /**
       * Open a directory for reading.  This uses the opendir(3) call to
       * open the underlying DIR stream.  Any previously open directory is
       * closed before opening the new one.  The dirstream error state is
       * set if the open fails, and an exception will be thrown.
       *
       * @param dirname the directory to read.
       * @see close()
       */
      void open(std::string const& dirname);

      /**
       * Close the directory.  This uses the closedir(3) call to close the
       * underlying DIR stream.  All cached data is deleted and the error
       * state set until open() is called.
       *
       * @see open()
       */
      void close();

      /**
       * Check for End Of File.  Note that the end of file status is
       * only set adter a read fails, so this should be checked after
       * each read.
       *
       * @returns true if the dirstream is empty, otherwise false.
       */
      bool eof() const;

      /**
       * Check for errors.  If there is an error, the dirstream is
       * unusable until the next open() call.
       *
       * @returns true if the dirstream is in an error state, otherwise
       * false.
       */
      bool bad() const;

      /**
       * Check if the dirstream status is good.
       *
       * @returns true if the status is good (eof() and bad() both
       * return false).
       */
      operator bool ();

      /**
       * Check if the dirstream status is bad.
       *
       * @returns true if the status is bad (eof() or bad() return
       * true).
       */
      bool
      operator ! ();

      friend dirstream&
      operator >> (dirstream& stream,
		   direntry&  entry);

    private:
      /**
       * Read dirents from the underlying DIR stream into the data
       * deque.  If the read fails, the error state will be set, and
       * an exception will be thrown.
       *
       * @param quantity the number of dirents to read.
       */
      void read (int quantity=1);

      /// The directory name.
      std::string dirname;

      /// The underlying DIR stream.
      DIR *dir;

      /**
       * A list of direntries represents the directory stream as a LIFO
       * stack.
       */
      std::deque<direntry> data;

      /// Error status.
      bool error_status;

      /// End of File status.
      bool eof_status;
  };

  /**
   * The overloaded extraction operator.  This is used to pull
   * direntries from a dirstream.
   *
   * @param stream the dirstream to get input from.
   * @param entry the direntry to set.
   * @returns the dirstream.
   */
  dirstream&
  operator >> (dirstream& stream,
	       direntry&  entry);

}

#endif /* SBUILD_DIRSTREAM_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
