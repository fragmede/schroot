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

#include "sbuild-chroot.h"
#include "sbuild-chroot-facet-unshare.h"
#include "sbuild-feature.h"

#include <boost/format.hpp>

#ifdef SBUILD_FEATURE_UNSHARE
#include <sched.h>
#endif // SBUILD_FEATURE_UNSHARE

using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::chroot_facet_unshare::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = integer personality ID
      emap(sbuild::chroot_facet_unshare::UNSHARE,
	   N_("Could not unshare process execution context"))
    };

#ifdef SBUILD_FEATURE_UNSHARE
  sbuild::feature feature_unshare
  ("UNSHARE",
   N_("Linux dissassociation of shared execution context"));
#endif

}

template<>
error<sbuild::chroot_facet_unshare::error_code>::map_type
error<sbuild::chroot_facet_unshare::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

chroot_facet_unshare::chroot_facet_unshare ():
  chroot_facet(),
  unshare_net(false)
{
}

chroot_facet_unshare::~chroot_facet_unshare ()
{
}

chroot_facet_unshare::ptr
chroot_facet_unshare::create ()
{
  return ptr(new chroot_facet_unshare());
}

chroot_facet::ptr
chroot_facet_unshare::clone () const
{
  return ptr(new chroot_facet_unshare(*this));
}

std::string const&
chroot_facet_unshare::get_name () const
{
  static const std::string name("unshare");

  return name;
}

bool
chroot_facet_unshare::get_unshare_net () const
{
  return this->unshare_net;
}

void
chroot_facet_unshare::set_unshare_net (bool unshare_net)
{
  this->unshare_net = unshare_net;
}

void
chroot_facet_unshare::unshare () const
{
  if (this->unshare_net)
    {
      log_debug(DEBUG_INFO) << "Unsharing network" << std::endl;
      if (::unshare(CLONE_NEWNET) < 0)
	throw error(UNSHARE, strerror(errno));
    }
}

void
chroot_facet_unshare::setup_env (chroot const& chroot,
				 environment&  env) const
{
  env.add("UNSHARE_NEWNET", get_unshare_net());
}

sbuild::chroot::session_flags
chroot_facet_unshare::get_session_flags (chroot const& chroot) const
{
  return sbuild::chroot::SESSION_NOFLAGS;
}

void
chroot_facet_unshare::get_details (chroot const&  chroot,
				   format_detail& detail) const
{
  detail.add(_("Unshare Networking"), get_unshare_net());
}

void
chroot_facet_unshare::get_used_keys (string_list& used_keys) const
{
  used_keys.push_back("unshare.newnet");
}

void
chroot_facet_unshare::get_keyfile (chroot const& chroot,
				   keyfile&      keyfile) const
{
  keyfile::set_object_value(*this, &chroot_facet_unshare::get_unshare_net,
			    keyfile, chroot.get_name(), "unshare.newnet");
}

void
chroot_facet_unshare::set_keyfile (chroot&        chroot,
				   keyfile const& keyfile)
{
  keyfile::get_object_value(*this, &chroot_facet_unshare::set_unshare_net,
			    keyfile, chroot.get_name(), "unshare.newnet",
			    keyfile::PRIORITY_OPTIONAL);
}
