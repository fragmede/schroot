/* Copyright © 2003,2006  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild-dirstream.h"

#include <cerrno>
#include <cstring>

using namespace sbuild;

namespace
{

  typedef std::pair<dirstream::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(dirstream::DIR_OPEN,    N_("Failed to open directory")),
      emap(dirstream::DIR_READ,    N_("Failed to read directory"))
    };

}

template<>
error<dirstream::error_code>::map_type
error<dirstream::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));


dirstream::dirstream(std::string const& dirname):
  dirname(),
  dir(0),
  data(),
  error_status(true),
  eof_status(true)
{
  open(dirname);
}


dirstream::~dirstream()
{
  close();
}

void
dirstream::open(std::string const& dirname)
{
  this->dir = opendir(dirname.c_str());
  if (this->dir == 0)
    {
      this->dirname.clear();
      this->error_status = true;
      this->eof_status = true;
      throw error(dirname, DIR_OPEN, strerror(errno));
    }
  this->dirname = dirname;
  this->error_status = false;
  this->eof_status = false;
  read();
}

void
dirstream::read(int quantity)
{
  int i;

  if (this->dir == 0)
    return;

  for (i = 0; i < quantity; ++i)
    {
      struct dirent* entry;
      errno = 0;
      entry = readdir(dir);

      if (entry == 0) // EOF or error
	{
	  //std::cerr << "Directory read error: ";
	  if (errno) // error
	    {
	      this->error_status = true;
	      throw error(this->dirname, DIR_READ, strerror(errno));
	    }
	  return;
	}

      direntry newentry(entry); // make a direntry
      this->data.push_back(newentry); // push onto the end of the list
    }
}

// close the directory
// this also clears all the direntry data
void
dirstream::close()
{
  if (this->dir)
    closedir(this->dir); // don't throw an exception on failure -- it could
		   // be called in the destructor
  this->dir = 0;
  this->data.clear();    // clear all data
  this->dirname.clear();
  this->error_status = true;
  this->eof_status = true;
}


bool
dirstream::eof() const
{
  return this->eof_status;
}

bool
dirstream::bad() const
{
  return this->error_status;
}

sbuild::dirstream::operator bool ()
{
  return !(bad() || eof());
}

bool
sbuild::dirstream::operator ! ()
{
  return bad() || eof();
}


dirstream&
sbuild::operator >> (dirstream& stream,
		     direntry&  entry)
{
  stream.read(); // read a new entry
  if (stream && !stream.data.empty()) // not at end of file or bad.
    {
      entry = stream.data.front(); // assign next direntry to entry
      stream.data.pop_front(); // remove the entry
    }
  else // blank the direntry and set EOF status
    {
      std::memset(&entry, 0, sizeof(direntry));
      stream.eof_status = true;
    }

  return stream;
}
