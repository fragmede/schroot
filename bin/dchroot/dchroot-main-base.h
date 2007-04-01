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

#ifndef DCHROOT_MAIN_BASE_H
#define DCHROOT_MAIN_BASE_H

#include <schroot/schroot-main-base.h>

namespace dchroot
{

  /**
   * Frontend base class for dchroot.  This class contains frontend
   * functionality common to dchroot and dchroot-dsa.
   */
  class main_base : public schroot::main_base
  {
  public:
    /**
     * The constructor.
     *
     * @param program_name the program name.
     * @param program_usage the program usage message.
     * @param options the command-line options to use.
     */
    main_base (std::string const& program_name,
	       std::string const& program_usage,
	       schroot::options_base::ptr& options);

    /// The destructor.
    virtual ~main_base ();

  protected:
    /**
     * Check dchroot.conf exists.  If it exists, and using verbose
     * messages, print a warning about upgrading to schroot.conf.
     */
    void
    check_dchroot_conf();

    virtual void
    compat_check ();

    virtual void
    action_config ();

    virtual void
    action_list ();

    /// Use dchroot.conf as the configuration file.
    bool use_dchroot_conf;
  };

}

#endif /* DCHROOT_MAIN_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
