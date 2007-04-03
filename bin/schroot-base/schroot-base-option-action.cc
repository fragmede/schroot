/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-i18n.h>

#include "schroot-base-option-action.h"

#include <iomanip>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using namespace schroot_base;

option_action::option_action ():
  default_action(),
  current_action(),
  actions()
{
}

option_action::~option_action ()
{
}

void
option_action::add_action (std::string const& action)
{
  this->actions.insert(action);
}

std::string const&
option_action::get_default_action ()
{
  return this->default_action;
}

void
option_action::set_default_action (std::string const& action)
{
  if (is_action(action))
    this->default_action = action;
  else
    throw std::logic_error((format(_("%1%: invalid action")) % action).str());
}

std::string const&
option_action::get_action ()
{
  if (this->current_action != "")
    return this->current_action;
  else
    return this->default_action;
}

void
option_action::set_action (std::string const& action)
{
  if (is_action(action))
    this->current_action = action;
  else
    throw std::logic_error((format(_("%1%: invalid action")) % action).str());
}

bool
option_action::is_action (std::string const& action)
{
  return this->actions.find(action) != this->actions.end();
}

