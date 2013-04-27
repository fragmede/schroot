/* Copyright © 2005-2013  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_FACET_DIRECTORY_BASE_H
#define SBUILD_CHROOT_FACET_DIRECTORY_BASE_H

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/facet/storage.h>

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      /**
       * A base class for block-device chroots.
       *
       * This class doesn't implement a chroot (get_chroot_type
       * is not implemented).
       *
       * Originally lvm-snapshot inherited from the block-device chroot,
       * but this was changed when union support was introduced.  This
       * design prevents lvm-snapshot offering union based sessions.
       */
      class directory_base : public storage
      {
      public:
        /// Exception type.
        typedef chroot::error error;

      protected:
        /// The constructor.
        directory_base ();

        /// The copy constructor.
        directory_base (const directory_base& rhs);

        friend class chroot;

      public:
        /// The destructor.
        virtual ~directory_base ();

      protected:
        void
        set_chroot (chroot& chroot);

      public:
        /**
         * Get the directory containing the chroot.
         *
         * @returns the location.
         */
        std::string const&
        get_directory () const;

        /**
         * Set the directory containing the chroot.
         *
         * @param directory the directory.
         */
        void
        set_directory (std::string const& directory);

        virtual std::string
        get_path () const;

        virtual void
        setup_env (chroot const& chroot,
                   environment&  env) const;

      protected:
        virtual void
        get_details (chroot const&  chroot,
                     format_detail& detail) const;

        virtual void
        get_used_keys (string_list& used_keys) const;

        virtual void
        get_keyfile (chroot const& chroot,
                     keyfile&      keyfile) const;

        virtual void
        set_keyfile (chroot&        chroot,
                     keyfile const& keyfile);

        /// The directory to use.
        std::string directory;
      };

    }
  }
}

#endif /* SBUILD_CHROOT_FACET_DIRECTORY_BASE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
