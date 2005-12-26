/* sbuild-config - sbuild config object
 *
 * Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CONFIG_H
#define SBUILD_CONFIG_H

#include <map>
#include <ostream>
#include <vector>
#include <string>

#include "sbuild-chroot.h"
#include "sbuild-error.h"

namespace sbuild
{

  class Config
  {
  public:
    typedef std::vector<Chroot *> chroot_list;
    typedef std::map<std::string, std::string> string_map;
    typedef std::map<std::string, Chroot *> chroot_map;

    enum ErrorCode
      {
	ERROR_STAT_FAIL,
	ERROR_OWNERSHIP,
	ERROR_PERMISSIONS,
	ERROR_NOT_REGULAR
      };

    typedef Exception<ErrorCode> error;

    Config();
    Config(const std::string& file);
    virtual ~Config();

    void
    add_config_file (const std::string& file);

    void
    add_config_directory (const std::string& dir);

    chroot_list
    get_chroots () const;

    const Chroot *
    find_chroot (const std::string& name) const;

    const Chroot *
    find_alias (const std::string& name) const;

    string_list
    get_chroot_list () const;

    void
    print_chroot_list (std::ostream& stream) const;

    void
    print_chroot_info (const string_list& chroots,
		       std::ostream&      stream) const;

    string_list
    validate_chroots(const string_list& chroots) const;

  private:
    void
    check_security(int fd) const;

    void
    load (const std::string& file);

    chroot_map chroots;
    string_map aliases;
  };

}

#endif /* SBUILD_CONFIG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
