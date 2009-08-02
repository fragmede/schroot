/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild-chroot-plain.h"
#include "sbuild-chroot-facet-session.h"
#include "sbuild-format-detail.h"
#include "sbuild-lock.h"

#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

using namespace sbuild;

chroot_plain::chroot_plain ():
  chroot_directory_base()
{
  set_run_setup_scripts(false);

  remove_facet<chroot_facet_session>();
}

chroot_plain::~chroot_plain ()
{
}

sbuild::chroot::ptr
chroot_plain::clone () const
{
  return ptr(new chroot_plain(*this));
}

chroot::ptr
sbuild::chroot_plain::clone_session (std::string const& session_id) const
{
  return ptr();
}

chroot::ptr
sbuild::chroot_plain::clone_source () const
{
  return ptr();
}

std::string const&
chroot_plain::get_chroot_type () const
{
  static const std::string type("plain");

  return type;
}

std::string
chroot_plain::get_path () const
{
  return get_directory();
}

void
chroot_plain::setup_lock (chroot::setup_type type,
			  bool               lock,
			  int                status)
{
  /* By default, plain chroots do no locking. */
}

sbuild::chroot::session_flags
chroot_plain::get_session_flags (chroot const& chroot) const
{
  return SESSION_NOFLAGS;
}
