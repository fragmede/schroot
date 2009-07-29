/* Copyright © 2005-2009  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-chroot-facet-personality.h"

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

chroot_facet_personality::chroot_facet_personality ():
  chroot_facet(),
  persona(
#ifdef __linux__
	  personality("linux")
#else
	  personality("undefined")
#endif
	  )
{
}

chroot_facet_personality::~chroot_facet_personality ()
{
}

chroot_facet_personality::ptr
chroot_facet_personality::create ()
{
  return ptr(new chroot_facet_personality());
}

chroot_facet::ptr
chroot_facet_personality::clone () const
{
  return ptr(new chroot_facet_personality(*this));
}

personality const&
chroot_facet_personality::get_persona () const
{
  return this->persona;
}

void
chroot_facet_personality::set_persona (personality const& persona)
{
  this->persona = persona;
}

void
chroot_facet_personality::setup_env (chroot const& chroot,
				     environment&  env) const
{
}

chroot::session_flags
chroot_facet_personality::get_session_flags (chroot const& chroot) const
{
  return sbuild::chroot::SESSION_NOFLAGS;
}

void
chroot_facet_personality::get_details (chroot const&  chroot,
				       format_detail& detail) const
{
  // TRANSLATORS: "Personality" is the Linux kernel personality
  // (process execution domain).  See schroot.conf(5).
  detail.add(_("Personality"), get_persona().get_name());
}

void
chroot_facet_personality::get_keyfile (chroot const& chroot,
				       keyfile&      keyfile) const
{
  keyfile::set_object_value(*this, &chroot_facet_personality::get_persona,
			    keyfile, chroot.get_keyfile_name(), "personality");
}

void
chroot_facet_personality::set_keyfile (chroot&        chroot,
				       keyfile const& keyfile,
				       string_list&   used_keys)
{
  keyfile::get_object_value(*this, &chroot_facet_personality::set_persona,
			    keyfile, chroot.get_keyfile_name(), "personality",
			    keyfile::PRIORITY_OPTIONAL);
  used_keys.push_back("personality");
}
