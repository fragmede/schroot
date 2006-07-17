/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
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

#include "sbuild-personality.h"

#include <cstring>
#include <cerrno>
#include <utility>

#ifdef __linux__
#include <sys/personality.h>
#endif

#include <boost/format.hpp>

using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::personality::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(sbuild::personality::BAD, N_("Personality '%1%' is unknown")),
      emap(sbuild::personality::SET, N_("Failed to set personality '%1%'"))
    };

  typedef std::pair<std::string,sbuild::personality::type> pmap;

  /**
   * This is a list of the supported personalities.  It's used to
   * construct the real personalities map.
   */
  pmap initial_personalities[] =
    {
      pmap("undefined", 0xffffffff),
#ifdef __linux__
      pmap("linux", PER_LINUX),
      pmap("linux_32bit", PER_LINUX_32BIT),
      pmap("svr4", PER_SVR4),
      pmap("scorvr3", PER_SCOSVR3),
      pmap("osr5", PER_OSR5),
      pmap("wysev386", PER_WYSEV386),
      pmap("iscr4", PER_ISCR4),
      pmap("bsd", PER_BSD),
      pmap("sunos", PER_SUNOS),
      pmap("xenix", PER_XENIX),
      pmap("linux32", PER_LINUX32),
      pmap("irix32", PER_IRIX32),
      pmap("irixn32", PER_IRIXN32),
      pmap("irix64", PER_IRIX64),
      pmap("riscos", PER_RISCOS),
      pmap("solaris", PER_SOLARIS),
      pmap("uw7", PER_UW7),
      pmap("hpux", PER_HPUX),
      pmap("osf4", PER_OSF4),
#endif
    };

}

template<>
error<sbuild::personality::error_code>::map_type
error<sbuild::personality::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

std::map<std::string,sbuild::personality::type>
sbuild::personality::personalities(initial_personalities,
				   initial_personalities + (sizeof(initial_personalities) / sizeof(initial_personalities[0])));

sbuild::personality::personality ():
  persona(
#ifdef __linux__
	  ::personality(0xffffffff)
#else
	  0xffffffff
#endif
	  )
{
}

sbuild::personality::personality (type persona):
  persona(persona)
{
}

sbuild::personality::personality (std::string const& persona):
  persona(find_personality(persona))
{
}

sbuild::personality::~personality ()
{
}

sbuild::personality::type
sbuild::personality::find_personality (std::string const& persona)
{
  std::map<std::string,type>::const_iterator pos =
    personalities.find(persona);

  if (pos != personalities.end())
    return pos->second;

  return 0xffffffff;
}

std::string const&
sbuild::personality::find_personality (type persona)
{
  static const std::string unknown("unknown");

  for (std::map<std::string,type>::const_iterator pos = personalities.begin();
       pos != personalities.end();
       ++pos)
    if (pos->second == persona)
      return pos->first;

  return unknown;
}

std::string const&
sbuild::personality::get_name () const
{
  return find_personality(this->persona);
}

sbuild::personality::type
sbuild::personality::get () const
{
  return this->persona;
}

void
sbuild::personality::set () const
{
#ifdef __linux__
  /* Set the process execution domain using personality(2). */
  if (this->persona != 0xffffffff &&
      ::personality (this->persona) < 0)
    {
      throw error(get_name(), SET, strerror(errno));
    }
#endif
}

std::string
sbuild::personality::get_personalities ()
{
  format fmt(_("Valid personalities: %1%\n"));
  std::string ps;

  for (std::map<std::string,type>::const_iterator pos = personalities.begin();
       pos != personalities.end();
       ++pos)
    {
      ps += pos->first;
      std::map<std::string,type>::const_iterator stpos = pos;
      if (++stpos != personalities.end())
	ps += ", ";
    }

  fmt % ps;

  return fmt.str();
}
