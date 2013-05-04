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

#ifndef SBUILD_CHROOT_FACET_SESSION_CLONABLE_H
#define SBUILD_CHROOT_FACET_SESSION_CLONABLE_H

#include <sbuild/chroot/facet/facet.h>

namespace sbuild
{
  namespace chroot
  {
    namespace facet
    {

      /**
       * Chroot support for creation of sessions.
       *
       * A chroot may offer a "session" facet to signal restorable or
       * parallel chroot environment usage.  This facet allows the chroot
       * to support session creation.
       */
      class session_clonable : public facet
      {
      public:
        /// A shared_ptr to a chroot facet object.
        typedef std::shared_ptr<session_clonable> ptr;

        /// A shared_ptr to a const chroot facet object.
        typedef std::shared_ptr<const session_clonable> const_ptr;

      private:
        /// The constructor.
        session_clonable ();

      public:
        /// The destructor.
        virtual ~session_clonable ();

        /**
         * Create a chroot facet.
         *
         * @returns a shared_ptr to the new chroot facet.
         */
        static ptr
        create ();

        virtual facet::ptr
        clone () const;

        virtual std::string const&
        get_name () const;

        /**
         * Set the defaults in the cloned session chroot.
         *
         * @param parent the parent of the cloned chroot.
         * @param clone the chroot to set up.
         * @param session_id the identifier for the new session.
         * @param user the user creating the session.
         * @param root whether or not the user is switching to root.
         */
        virtual chroot::ptr
        clone_session (std::string const& session_id,
                       std::string const& alias,
                       std::string const& user,
                       bool               root) const;

        virtual chroot::session_flags
        get_session_flags (chroot const& chroot) const;
      };

    }
  }
}

#endif /* SBUILD_CHROOT_FACET_SESSION_CLONABLE_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
