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

#ifndef SBUILD_CHROOT_FACET_BTRFS_SNAPSHOT_H
#define SBUILD_CHROOT_FACET_BTRFS_SNAPSHOT_H

#include <sbuild/chroot/chroot.h>
#include <sbuild/chroot/facet/storage.h>

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      /**
       * A chroot stored on an BTRFS subvolume.
       *
       * A snapshot subvolume will be created and mounted on demand.
       */
      class btrfs_snapshot : public storage
      {
      public:
        /// Exception type.
        typedef chroot::error error;

        /// A shared_ptr to a chroot facet object.
        typedef std::shared_ptr<btrfs_snapshot> ptr;

        /// A shared_ptr to a const chroot facet object.
        typedef std::shared_ptr<const btrfs_snapshot> const_ptr;

      protected:
        /// The constructor.
        btrfs_snapshot ();

        /// The copy constructor.
        btrfs_snapshot (const btrfs_snapshot& rhs);

        void
        set_chroot (chroot& chroot);

        friend class chroot;

      public:
        /// The destructor.
        virtual ~btrfs_snapshot ();

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
         * Get the source subvolume path.  This is used by "btrfs
         * subvolume snapshot".
         *
         * @returns the source subvolume.
         */
        std::string const&
        get_source_subvolume () const;

        /**
         * Set the source subvolume path.  This is used by "btrfs
         * subvolume snapshot".
         *
         * @param source_subvolume the source subvolume.
         */
        void
        set_source_subvolume (std::string const& source_subvolume);

        /**
         * Get the snapshot directory.
         *
         * @returns the directory.
         */
        std::string const&
        get_snapshot_directory () const;

        /**
         * Set the snapshot directory.
         *
         * @param snapshot_directory the snapshot directory.
         */
        void
        set_snapshot_directory (std::string const& snapshot_directory);

        /**
         * Get the snapshot name.  This is used by "btrfs subvolume
         * snapshot", and is the full path to the snapshot.
         *
         * @returns the name.
         */
        std::string const&
        get_snapshot_name () const;

        /**
         * Set the snapshot name.  This is used by "btrfs subvolume
         * snapshot", and is the full path to the snapshot.
         *
         * @param snapshot_name the snapshot name.
         */
        void
        set_snapshot_name (std::string const& snapshot_name);

        virtual std::string
        get_path () const;

        virtual void
        setup_env (chroot const& chroot,
                   environment&  env) const;

        virtual chroot::session_flags
        get_session_flags (chroot const& chroot) const;

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
        /// Btrfs source subvolume
        std::string source_subvolume;
        /// Btrfs snapshot path
        std::string snapshot_directory;
        /// Btrfs snapshot name
        std::string snapshot_name;
      };

    }
  }
}

#endif /* SBUILD_CHROOT_FACET_BTRFS_SNAPSHOT_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
