/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_CONFIG_H
#define SBUILD_CHROOT_CONFIG_H

#include <sbuild/sbuild-chroot.h>
#include <sbuild/sbuild-custom-error.h>

#include <map>
#include <ostream>
#include <vector>
#include <string>

namespace sbuild
{

  /**
   * Chroot configuration.
   *
   * This class holds the configuration details from the configuration
   * file.  Conceptually, it's an opaque container of chroot objects.
   *
   * Methods are provided to query the available chroots and find
   * specific chroots.
   */
 class chroot_config
  {
  public:
    /// A list of chroots.
    typedef std::vector<chroot::ptr> chroot_list;
    /// A map between key-value string pairs.
    typedef std::map<std::string, std::string> string_map;
    /// A map between a chroot name and a chroot object.
    typedef std::map<std::string, chroot::ptr> chroot_map;

    /// Error codes.
    enum error_code
      {
	ALIAS_EXIST,     ///< Alias already associated with chroot.
	CHROOT_NOTFOUND, ///< No such chroot.
	CHROOT_EXIST,    ///< A chroot or alias already exists with this name.
	FILE_NOTREG,     ///< File is not a regular file.
	FILE_OPEN,       ///< Failed to open file.
	FILE_OWNER,      ///< File is not owned by user root.
	FILE_PERMS       ///< File has write permissions for others.
      };

    /// Exception type.
    typedef custom_error<error_code> error;

    /// A shared_ptr to a chroot_config object.
    typedef std::tr1::shared_ptr<chroot_config> ptr;

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

    /**
     * Add a configuration file or directory.  The configuration file
     * or directory specified will be loaded.
     *
     * @param location initialise using a configuration file or a
     * whole directory containing configuration files.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    void
    add (std::string const& location,
	 bool               active);

  private:
    /**
     * Add a configuration file.  The configuration file specified
     * will be loaded.
     *
     * @param file the file to load.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    void
    add_config_file (std::string const& file,
		     bool               active);

    /**
     * Add a configuration directory.  The configuration files in the
     * directory specified will all be loaded.
     *
     * @param dir the directory containing the files to load.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    void
    add_config_directory (std::string const& dir,
			  bool               active);

  protected:
    /**
     * Add a chroot.  The lists of chroots and aliases will be
     * updated.  If a chroot or alias by the same name exists, the
     * chroot will not be added, and a warning will be logged.  Af any
     * of the aliases already exist, a warning will be logged, and the
     * alias will not be added.
     *
     * @param chroot the chroot to add.
     * @param kconfig the chroot configuration.
     */
    void
    add (chroot::ptr&   chroot,
	 keyfile const& kconfig);

  public:
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
    const chroot::ptr
    find_chroot (std::string const& name) const;

    /**
     * Find a chroot by its name or an alias.
     *
     * @param name the chroot name or alias.
     * @returns the chroot if found, otherwise 0.
     */
    const chroot::ptr
    find_alias (std::string const& name) const;

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
     * Print a single line of all the available chroots to the
     * specified stream.
     *
     * @param stream the stream to output to.
     */
    void
    print_chroot_list_simple (std::ostream& stream) const;

    /**
     * Print information about the specified chroots to the specified
     * stream.
     *
     * @param chroots a list of chroots to print.
     * @param stream the stream to output to.
     */
    void
    print_chroot_info (string_list const& chroots,
		       std::ostream&      stream) const;

    /**
     * Print location information about the specified chroots to the
     * specified stream.
     *
     * @param chroots a list of chroots to print.
     * @param stream the stream to output to.
     */
    void
    print_chroot_location (string_list const& chroots,
			   std::ostream&      stream) const;

    /**
     * Print configuration of the specified chroots to the specified
     * stream.
     *
     * @param chroots a list of chroots to print.
     * @param stream the stream to output to.
     */
    void
    print_chroot_config (string_list const& chroots,
			 std::ostream&      stream) const;

    /**
     * Check that all the chroots specified exist.
     *
     * @param chroots a list of chroots to validate.
     * @returns a list of invalid chroots.  The list will be empty if
     * all chroots are valid.
     */
    string_list
    validate_chroots (string_list const& chroots) const;

  private:
    /**
     * Load a configuration file.  If there are problems with the
     * configuration file, an error will be thrown.  The file must be
     * owned by root, not writable by other, and be a regular file.
     *
     * @param file the file to load.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    void
    load_data (std::string const& file,
	       bool               active);

  protected:
    /**
     * Parse a loaded configuration file.  If there are problems with
     * the configuration file, an error will be thrown.
     *
     * @param stream the data stream to parse.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    virtual void
    parse_data (std::istream& stream,
		bool          active);

    /**
     * Load a keyfile.  If there are problems with the configuration
     * file, an error will be thrown.
     *
     * @param kconfig the chroot configuration.
     * @param active true if the chroots in the configuration file are
     * active sessions, otherwise false.
     */
    virtual void
    load_keyfile (keyfile& kconfig,
		  bool     active);

    /// A list of chroots (name->chroot mapping).
    chroot_map chroots;
    /// A list of aliases (alias->name mapping).
    string_map aliases;
  };

}

#endif /* SBUILD_CHROOT_CONFIG_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
