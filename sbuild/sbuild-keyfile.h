/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_KEYFILE_H
#define SBUILD_KEYFILE_H

#include <sbuild/sbuild-basic-keyfile.h>

namespace sbuild
{

  /**
   * Traits class for an INI-style configuration file.  The format is
   * documented in schroot.conf(5).  It is an independent
   * reimplementation of the Glib GKeyFile class, which it replaces.
   */
  struct keyfile_traits
  {
    /// Group name.
    typedef std::string group_name_type;

    /// Key name.
    typedef std::string key_type;

    /// Value.
    typedef std::string value_type;

    /// Comment.
    typedef std::string comment_type;

    /// Line number.
    typedef unsigned int size_type;
  };

  /**
   * Configuration file parser.  This class loads an INI-style
   * configuration file from a file or stream.  The format is
   * documented in schroot.conf(5).  It is an independent
   * reimplementation of the Glib GKeyFile class, which it replaces.
   */
  typedef sbuild::basic_keyfile<keyfile_traits> keyfile;

}

#endif /* SBUILD_KEYFILE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
