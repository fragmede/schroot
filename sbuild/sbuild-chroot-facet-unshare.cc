/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#ifdef SBUILD_FEATURE_UNSHARE
  sbuild::feature feature_unshare
  ("UNSHARE",
   N_("Linux dissassociation of shared execution context"));
#endif

}

template<>
error<sbuild::chroot_facet_unshare::error_code>::map_type
error<sbuild::chroot_facet_unshare::error_code>::error_strings =
  {
    // TRANSLATORS: %1% = the name of the context being unshared
    {sbuild::chroot_facet_unshare::UNSHARE,
     N_("Could not unshare ‘%1%’ process execution context")}
  };

chroot_facet_unshare::chroot_facet_unshare ():
  chroot_facet(),
  unshare_net(false),
  unshare_sysvipc(false),
  unshare_sysvsem(false),
  unshare_uts(false)
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

bool
chroot_facet_unshare::get_unshare_sysvipc () const
{
  return this->unshare_sysvipc;
}

void
chroot_facet_unshare::set_unshare_sysvipc (bool unshare_sysvipc)
{
  this->unshare_sysvipc = unshare_sysvipc;
}

bool
chroot_facet_unshare::get_unshare_sysvsem () const
{
  return this->unshare_sysvsem;
}

void
chroot_facet_unshare::set_unshare_sysvsem (bool unshare_sysvsem)
{
  this->unshare_sysvsem = unshare_sysvsem;
}

bool
chroot_facet_unshare::get_unshare_uts () const
{
  return this->unshare_uts;
}

void
chroot_facet_unshare::set_unshare_uts (bool unshare_uts)
{
  this->unshare_uts = unshare_uts;
}

void
chroot_facet_unshare::unshare () const
{
#ifdef CLONE_NEWNET
  if (this->unshare_net)
    {
      log_debug(DEBUG_INFO) << "Unsharing network" << std::endl;
      if (::unshare(CLONE_NEWNET) < 0)
        throw error("NET", UNSHARE, strerror(errno));
    }
#endif
#ifdef CLONE_NEWIPC
  if (this->unshare_sysvipc)
    {
      log_debug(DEBUG_INFO) << "Unsharing System V IPC" << std::endl;
      if (::unshare(CLONE_NEWIPC) < 0)
        throw error("SYSVIPC", UNSHARE, strerror(errno));
    }
#endif
#ifdef CLONE_SYSVSEM
  if (this->unshare_sysvsem)
    {
      log_debug(DEBUG_INFO) << "Unsharing System V SEM" << std::endl;
      if (::unshare(CLONE_SYSVSEM) < 0)
        throw error("SYSVSEM", UNSHARE, strerror(errno));
    }
#endif
#ifdef CLONE_UTS
  if (this->unshare_uts)
    {
      log_debug(DEBUG_INFO) << "Unsharing UTS namespace" << std::endl;
      if (::unshare(CLONE_UTS) < 0)
        throw error("UTS", UNSHARE, strerror(errno));
    }
#endif
}

void
chroot_facet_unshare::setup_env (chroot const& chroot,
                                 environment&  env) const
{
  env.add("UNSHARE_NET", get_unshare_net());
  env.add("UNSHARE_SYSVIPC", get_unshare_sysvipc());
  env.add("UNSHARE_SYSVSEM", get_unshare_sysvsem());
  env.add("UNSHARE_UTS", get_unshare_uts());
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
  detail.add(_("Unshare System V IPC"), get_unshare_sysvipc());
  detail.add(_("Unshare System V Semaphores"), get_unshare_sysvsem());
  detail.add(_("Unshare UTS namespace"), get_unshare_uts());
}

void
chroot_facet_unshare::get_used_keys (string_list& used_keys) const
{
  used_keys.push_back("unshare.net");
  used_keys.push_back("unshare.sysvipc");
  used_keys.push_back("unshare.sysvsem");
  used_keys.push_back("unshare.uts");
}

void
chroot_facet_unshare::get_keyfile (chroot const& chroot,
                                   keyfile&      keyfile) const
{
  keyfile::set_object_value(*this, &chroot_facet_unshare::get_unshare_net,
                            keyfile, chroot.get_name(), "unshare.net");
  keyfile::set_object_value(*this, &chroot_facet_unshare::get_unshare_sysvipc,
                            keyfile, chroot.get_name(), "unshare.sysvipc");
  keyfile::set_object_value(*this, &chroot_facet_unshare::get_unshare_sysvsem,
                            keyfile, chroot.get_name(), "unshare.sysvsem");
  keyfile::set_object_value(*this, &chroot_facet_unshare::get_unshare_uts,
                            keyfile, chroot.get_name(), "unshare.uts");
}

void
chroot_facet_unshare::set_keyfile (chroot&        chroot,
                                   keyfile const& keyfile)
{
  keyfile::get_object_value(*this, &chroot_facet_unshare::set_unshare_net,
                            keyfile, chroot.get_name(), "unshare.net",
                            keyfile::PRIORITY_OPTIONAL);
  keyfile::get_object_value(*this, &chroot_facet_unshare::set_unshare_sysvipc,
                            keyfile, chroot.get_name(), "unshare.sysvipc",
                            keyfile::PRIORITY_OPTIONAL);
  keyfile::get_object_value(*this, &chroot_facet_unshare::set_unshare_sysvsem,
                            keyfile, chroot.get_name(), "unshare.sysvsem",
                            keyfile::PRIORITY_OPTIONAL);
  keyfile::get_object_value(*this, &chroot_facet_unshare::set_unshare_uts,
                            keyfile, chroot.get_name(), "unshare.uts",
                            keyfile::PRIORITY_OPTIONAL);
}
