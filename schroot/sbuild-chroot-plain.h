/* sbuild-chroot-plain - sbuild simple chroot object
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

#ifndef SBUILD_CHROOT_PLAIN_H
#define SBUILD_CHROOT_PLAIN_H

#include "sbuild-chroot.h"

namespace sbuild
{

  class ChrootPlain : public Chroot
  {
  public:
    ChrootPlain();
    ChrootPlain (const keyfile&     keyfile,
		 const std::string& group);
    virtual ~ChrootPlain();

    virtual Chroot *
    clone () const;

    const std::string&
    get_location () const;

    void
    set_location (const std::string& location);

    virtual const std::string&
    get_mount_location () const;

    virtual const std::string&
    get_chroot_type () const;

    virtual void
    setup_env (env_list& env);

    virtual void
    setup_lock (SetupType type,
		bool      lock);

    virtual SessionFlags
    get_session_flags () const;

    virtual void
    print_details (std::ostream& stream) const;

    virtual void
    print_config (std::ostream& stream) const;

  private:
    void
    read_keyfile (const keyfile&     keyfile,
		  const std::string& group);

    std::string location;
  };

}

#endif /* SBUILD_CHROOT_PLAIN_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
