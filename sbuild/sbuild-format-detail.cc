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

#include "sbuild-format-detail.h"
#include "sbuild-i18n.h"

using namespace sbuild;

template<>
std::ostream&
sbuild::operator << (std::ostream&              stream,
		     format_detail<bool> const& rhs)
{
  const char *desc = 0;
  if (rhs.value)
    desc =  _("true");
  else
    desc = _("false");
  return stream << format_detail<std::string>(rhs.name, desc);
}
