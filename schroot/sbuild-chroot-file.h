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

#ifndef SBUILD_CHROOT_FILE_H
#define SBUILD_CHROOT_FILE_H

#include "sbuild-chroot.h"

namespace sbuild
{

  /**
   * A chroot stored in a file archive (tar or zip).  The archive will
   * be unpacked on demand.
   *
   * @todo Optionally mount and then unpack onto a tmpfs, to save
   * using a lot of space under /var.  However, this will require
   * careful checking and/or restrictions in order to prevent resource
   * starvation (using all swap space, for example).
   */
  class ChrootFile : public Chroot
  {
  protected:
    /// The constructor.
    ChrootFile();

    /**
     * The constructor.  Initialise from an open keyfile.
     *
     * @param keyfile the configuration file
     * @param group the keyfile group (chroot name)
     */
    ChrootFile (keyfile const&     keyfile,
		       std::string const& group);

    friend class Chroot;

  public:
    /// The destructor.
    virtual ~ChrootFile();

    virtual Chroot::chroot_ptr
    clone () const;

    /**
     * Get the file used by the chroot.
     *
     * @returns the file.
     */
    std::string const&
    get_file () const;

    /**
     * Set the file used by the chroot.
     *
     * @param file the file.
     */
    void
    set_file (std::string const& file);

    virtual std::string const&
    get_chroot_type () const;

    virtual void
    setup_env (environment& env);

    virtual void
    setup_lock (SetupType type,
		bool      lock);

    virtual SessionFlags
    get_session_flags () const;

  protected:
    virtual void
    print_details (std::ostream& stream) const;

    virtual void
    get_keyfile (keyfile& keyfile) const;

    virtual void
    set_keyfile (keyfile const& keyfile);

  private:
    /// The file to use.
    std::string file;
  };

}

#endif /* SBUILD_CHROOT_FILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
