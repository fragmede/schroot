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

#ifndef SBUILD_CHROOT_FACET_LOOPBACK_H
#define SBUILD_CHROOT_FACET_LOOPBACK_H

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/facet/storage.h>

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      /**
       * A chroot stored in a file for loopback mounting.
       *
       * The file will be mounted on demand.
       */
      class loopback : public storage
      {
      public:
        /// Exception type.
        typedef chroot::error error;

        /// A shared_ptr to a chroot facet object.
        typedef std::shared_ptr<loopback> ptr;

        /// A shared_ptr to a const chroot facet object.
        typedef std::shared_ptr<const loopback> const_ptr;

      protected:
        /// The constructor.
        loopback ();

        /// The copy constructor.
        loopback (const loopback& rhs);

        void
        set_chroot (chroot& chroot);

        friend class chroot;

      public:
        /// The destructor.
        virtual ~loopback ();

        virtual std::string const&
        get_name () const;

        /**
         * Create a chroot facet.
         *
         * @returns a shared_ptr to the new chroot facet.
         */
        static ptr
        create ();

        facet::ptr
        clone () const;

        /**
         * Get the filename containing the chroot.
         *
         * @returns the filename.
         */
        std::string const&
        get_filename () const;

        /**
         * Set the filename containing the chroot.
         *
         * @param filename the filename.
         */
        void
        set_filename (std::string const& filename);

        virtual std::string
        get_path () const;

        virtual void
        setup_env (chroot const& chroot,
                   environment&  env) const;

      protected:
        virtual void
        setup_lock (chroot::setup_type type,
                    bool               lock,
                    int                status);

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

      private:
        /// The file to use.
        std::string filename;
      };

    }
  }
}

#endif /* SBUILD_CHROOT_FACET_LOOPBACK_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
