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

chroot_file::chroot_file ():
  chroot(),
  chroot_source(),
  file(),
  repack(false)
{
  set_run_setup_scripts(true);
  set_run_exec_scripts(true);
}

chroot_file::~chroot_file ()
{
}

sbuild::chroot::ptr
chroot_file::clone () const
{
  return ptr(new chroot_file(*this));
}

sbuild::chroot::ptr
chroot_file::clone_source () const
{
  chroot_file *clone_file = new chroot_file(*this);
  ptr clone(clone_file);

  chroot_source::clone_source_setup(clone);
  clone_file->repack = true;

  return clone;
}

std::string const&
chroot_file::get_file () const
{
  return this->file;
}

void
chroot_file::set_file (std::string const& file)
{
  this->file = file;
}

std::string const&
chroot_file::get_chroot_type () const
{
  static const std::string type("file");

  return type;
}

void
chroot_file::setup_env (environment& env)
{
  chroot::setup_env(env);
  chroot_source::setup_env(env);

  env.add("CHROOT_FILE", get_file());
  env.add("CHROOT_FILE_REPACK", this->repack);
}

void
chroot_file::setup_lock (setup_type type,
			 bool       lock,
			 int        status)
{
  // Check ownership and permissions.
  if (type == SETUP_START && lock == true)
    {
      struct stat statbuf;
      if (stat(this->file.c_str(), &statbuf) < 0)
	throw error(this->file, FILE_STAT, errno);

      // NOTE: taken from chroot_config::check_security.
      if (statbuf.st_uid != 0)
	throw error(this->file, FILE_OWNER);
      if (statbuf.st_mode & S_IWOTH)
	throw error(this->file, FILE_PERMS);
      if (!S_ISREG(statbuf.st_mode))
	throw error(this->file, FILE_NOTREG);
    }

  /* By default, file chroots do no locking. */
  /* Create or unlink session information. */
  if ((type == SETUP_START && lock == true) ||
      (type == SETUP_STOP && lock == false && status == 0))
    {

      bool start = (type == SETUP_START);
      setup_session_info(start);
    }
}

sbuild::chroot::session_flags
chroot_file::get_session_flags () const
{
  return SESSION_CREATE;
}

void
chroot_file::print_details (std::ostream& stream) const
{
  chroot::print_details(stream);
  chroot_source::print_details(stream);

  if (!this->file.empty())
    stream << format_details(_("File"), get_file());
    stream << format_details(_("File Repack"), this->repack);
  stream << std::flush;
}

void
chroot_file::get_keyfile (keyfile& keyfile) const
{
  chroot::get_keyfile(keyfile);
  chroot_source::get_keyfile(keyfile);

  keyfile.set_value(get_name(), "file",
		    get_file());

  keyfile.set_value(get_name(), "file-repack",
		    this->repack);
}

void
chroot_file::set_keyfile (keyfile const& keyfile)
{
  chroot::set_keyfile(keyfile);
  chroot_source::set_keyfile(keyfile);

  std::string file;
  if (keyfile.get_value(get_name(), "file",
			keyfile::PRIORITY_REQUIRED, file))
    set_file(file);

  keyfile.get_value(get_name(), "file-repack",
		    get_active() ?
		    keyfile::PRIORITY_REQUIRED : keyfile::PRIORITY_DISALLOWED,
		    this->repack);
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
