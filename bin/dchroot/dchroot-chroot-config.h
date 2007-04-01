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

#ifndef DCHROOT_CHROOT_CONFIG_H
#define DCHROOT_CHROOT_CONFIG_H

#include <sbuild/sbuild-chroot-config.h>

namespace dchroot
{

  /**
   * Chroot configuration for dchroot compatibility.
   *
   * This class provides all the functionality of chroot_config, but
   * parses the dchroot configuration file format, rather than the
   * schroot format.
   */
  class chroot_config : public sbuild::chroot_config
  {
  public:
    /// The constructor.
    chroot_config ();

    /**
     * The constructor.
     *
     * @param file initialise using a configuration file or a whole
     * directory containing configuration files.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    chroot_config (std::string const& file,
		   bool               active);

    /// The destructor.
    virtual ~chroot_config ();

  private:
    virtual void
    parse_data (std::istream& stream,
		bool          active);
  };

}

#endif /* DCHROOT_CHROOT_CONFIG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
