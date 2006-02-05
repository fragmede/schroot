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

#include <config.h>

#include "sbuild.h"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

ChrootFile::ChrootFile():
  chroot(),
  file()
{
}

ChrootFile::ChrootFile (keyfile const&     keyfile,
			std::string const& group):
  chroot(keyfile, group),
  file()
{
}

ChrootFile::~ChrootFile()
{
}

sbuild::chroot::chroot_ptr
ChrootFile::clone () const
{
  return chroot_ptr(new ChrootFile(*this));
}

std::string const&
ChrootFile::get_file () const
{
  return this->file;
}

void
ChrootFile::set_file (std::string const& file)
{
  this->file = file;
}

std::string const&
ChrootFile::get_chroot_type () const
{
  static const std::string type("file");

  return type;
}

void
ChrootFile::setup_env (environment& env)
{
  this->chroot::setup_env(env);

  env.add("CHROOT_FILE", get_file());
}

void
ChrootFile::setup_lock (SetupType type,
			bool      lock)
{
  /* By default, file chroots do no locking. */
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false))
    {
      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::SessionFlags
ChrootFile::get_session_flags () const
{
  return SESSION_CREATE;
}

void
ChrootFile::print_details (std::ostream& stream) const
{
  this->chroot::print_details(stream);

  if (!this->file.empty())
    stream << format_details(_("File"), get_file());
  stream << std::flush;
}

void
ChrootFile::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);

  keyfile.set_value(get_name(), "file",
		    get_file());
}

void
ChrootFile::set_keyfile (keyfile const& keyfile)
{
  chroot::set_keyfile(keyfile);

  std::string file;
  if (keyfile.get_value(get_name(), "file",
			keyfile::PRIORITY_REQUIRED, file))
    set_file(file);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
