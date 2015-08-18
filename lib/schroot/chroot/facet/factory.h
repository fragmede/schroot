/* Copyright © 2011-2013  Roger Leigh <rleigh@codelibre.net>
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

#ifndef SCHROOT_CHROOT_FACET_FACTORY_H
#define SCHROOT_CHROOT_FACET_FACTORY_H

#include <schroot/chroot/chroot.h>
#include <schroot/chroot/facet/facet.h>

#include <map>
#include <ostream>
#include <string>

namespace schroot
{
  namespace chroot
  {
    namespace facet
    {

      class factory
      {
      public:
        struct facet_info
        {
          /// Facet name.
          std::string  name;
          /// Facet description.
          std::string  description;
          /// Install in a chroot automatically on chroot creation.
          bool         auto_install;
          /// Function to create an instance of this facet.
          facet::ptr (*create)();
        };

        factory (const facet_info& info);

        ~factory ();

        static std::ostream&
        print_facets (std::ostream& stream);

        static facet::ptr
        create (const std::string& name);

        static std::vector<facet::ptr>
        create_auto ();

      private:
        typedef std::map<std::string,const facet_info *> map_type;

        static map_type&
        registered_facets ();
      };

    }
  }
}

#endif /* SCHROOT_CHROOT_FACET_FACTORY_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
