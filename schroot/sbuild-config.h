/* Copyright © 2005  Roger Leigh <rleigh@debian.org>
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

  /**
   * Chroot configuration.
   *
   * This class holds the configuration details from the configuration
   * file.  Conceptually, it's an opaque container of Chroot objects.
   *
   * Methods are provided to query the available chroots and find
   * specific chroots.
   */
 class Config
  {
  public:
    /// A list of chroots.
    typedef std::vector<Chroot *> chroot_list;
    /// A map between key-value string pairs.
    typedef std::map<std::string, std::string> string_map;
    /// A map between a chroot name and a Chroot object.
    typedef std::map<std::string, Chroot *> chroot_map;

    /// Exception type.
    typedef runtime_error_custom<Config> error;

    /// The constructor.
    Config();

    /**
     * The constructor.
     *
     * @param file initialise using a configuration file or a whole
     * directory containing configuration files.
     */
    Config(const std::string& file);

    /// The destructor.
    virtual ~Config();

    /**
     * Add a configuration file.  The configuration file specified
     * will be loaded.
     *
     * @param file the file to load.
     */
    void
    add_config_file (const std::string& file);

    /**
     * Add a configuration directory.  The configuration files in the
     * directory specified will all be loaded.
     *
     * @param dir the directory containing the files to load.
     */
    void
    add_config_directory (const std::string& dir);

    /**
     * Get a list of available chroots.
     *
     * @returns a list of available chroots.  The list will be empty
     * if no chroots are available.
     */
    chroot_list
    get_chroots () const;

    /**
     * Find a chroot by its name.
     *
     * @param name the chroot name
     * @returns the chroot if found, otherwise 0.
     */
    const Chroot *
    find_chroot (const std::string& name) const;

    /**
     * Find a chroot by its name or an alias.
     *
     * @param name the chroot name or alias.
     * @returns the chroot if found, otherwise 0.
     */
    const Chroot *
    find_alias (const std::string& name) const;

    /**
     * Get the names (including aliases) of all the available chroots,
     * sorted in alphabetical order.
     *
     * @returns the list.  The list will be empty if no chroots are
     * available.
     */
    string_list
    get_chroot_list () const;

    /**
     * Print all the available chroots to the specified stream.
     *
     * @param stream the stream to output to.
     */
    void
    print_chroot_list (std::ostream& stream) const;

    /**
     * Print information about the specified chroots to the specified
     * stream.
     *
     * @param chroots a list of chroots to print.
     * @param stream the stream to output to.
     */
    void
    print_chroot_info (const string_list& chroots,
		       std::ostream&      stream) const;

    /**
     * Check that all the chroots specified exist.
     *
     * @param chroots a list of chroots to validate.
     * @returns a list of invalid chroots.  The list will be empty if
     * all chroots are valid.
     */
    string_list
    validate_chroots(const string_list& chroots) const;

  private:
    /**
     * Check the permissions and ownership of a configuration file.
     * The file must be owned by root, not writable by other, and be a
     * regular file.
     *
     * An error will be thrown on failure.
     *
     * @param fd the file descriptor to check.
     */
    void
    check_security(int fd) const;

    /**
     * Load a configuration file.  If there are problems with the
     * configuration file, the program will be aborted immediately.
     *
     * @param file the file to load.
     */
    void
    load (const std::string& file);

    /// A list of chroots (name->chroot mapping).
    chroot_map chroots;
    /// A list of aliases (alias->name mapping).
    string_map aliases;
  };

}

#endif /* SBUILD_CONFIG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
