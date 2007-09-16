/* Copyright © 2006-2007  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/sbuild-i18n.h>

#include "schroot-base-option-action.h"

#include <iomanip>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

using std::endl;
using boost::format;
using sbuild::_;
namespace opt = boost::program_options;
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
option_action::add (action_type const& action)
{
  this->actions.insert(action);
}

option_action::action_type const&
option_action::get_default ()
{
  return this->default_action;
}

void
option_action::set_default (action_type const& action)
{
  if (valid(action))
    this->default_action = action;
  else
    throw std::logic_error((format(_("%1%: invalid action")) % action).str());
}

option_action::action_type const&
option_action::get ()
{
  if (this->current_action != "")
    return this->current_action;
  else
    return this->default_action;
}

void
option_action::set (action_type const& action)
{
  if (valid(action))
    {
      if (this->current_action == "")
	this->current_action = action;
      else
	throw opt::validation_error(_("Only one action may be specified"));
    }
  else
    throw std::logic_error((format(_("%1%: invalid action")) % action).str());
}

bool
option_action::valid (action_type const& action)
{
  return this->actions.find(action) != this->actions.end();
}

