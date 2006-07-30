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

#include "sbuild-types.h"

using namespace sbuild;

const char *
sbuild::date_base::get_date_format ()
{
  // TRANSLATORS: Format string for date representation:
  // %d = day (number, e.g. 14)
  // %b = month (three letters, e.g. Jul)
  // %Y = year (four digits, e.g. 2006)
  // If required, any of the standard strftime(3)
  // format specifiers may be used instead, as long as
  // the day, month and year are clearly displayed in
  // the equivalent standard method for your locale.
  return _("%d %b %Y");
}
