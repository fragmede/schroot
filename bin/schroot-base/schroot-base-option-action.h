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

#ifndef SCHROOT_BASE_OPTION_ACTION_H
#define SCHROOT_BASE_OPTION_ACTION_H

#include <set>
#include <string>

namespace schroot_base
{

  /**
   * Actions specified as command-line options.  This class contains
   * all allowed options.  This replaced the use of enums to allow
   * extension of the options by inheritance.
   *
   * @todo Construct from iterator pair.
   * @todo: Throw logic_error if an invalid action is set.
   */
  class option_action
  {
  public:
    /// The constructor.
    option_action ();

    /// The destructor.
    virtual ~option_action ();

    /**
     * Add an action.  The specified action is added to the list of
     * permitted actions.
     * @param action the action to add.
     */
    void
    add_action (std::string const& action);

    /*
     * Get the default action.
     * @returns the default action, or an empty string if no default
     * action has been set.
     */
    std::string const&
    get_default_action ();

    /**
     * Set the default action.
     * @param action the action to set.
     */
     void
    set_default_action (std::string const& action);

    /*
     * Get the action to perform.
     * @returns the action, or the default action if no action has
     * been set, or an empty string if no default action has been set.
     */
    std::string const&
    get_action ();

    /**
     * Set the action to perform.
     * @param action the action to set.
     */
    void
    set_action (std::string const& action);

    /**
     * Check if an action is valid.
     * @param action the action to check.
     * @returns if action is a valid action, otherwise false.
     */
    bool
    is_action (std::string const& action);

  private:
    /// The container of the actions.
    typedef std::set<std::string> action_set;

    /// Default action.
    std::string default_action;

    /// Current action.
    std::string current_action;

    /// Allowed actions.
    action_set  actions;
  };

}

#endif /* SCHROOT_BASE_OPTION_ACTION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
