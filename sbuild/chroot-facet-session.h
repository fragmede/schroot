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

#ifndef SBUILD_CHROOT_FACET_SESSION_H
#define SBUILD_CHROOT_FACET_SESSION_H

#include <sbuild/chroot-facet.h>
#include <sbuild/session.h>

namespace sbuild
{

  /**
   * Chroot support for sessions.
   *
   * A chroot may offer a "session" facet to signal restorable or
   * parallel chroot environment usage.  The presence of this facet
   * indicates that the chroot is an active session.
   */
  class chroot_facet_session : public chroot_facet
  {
  public:
    /// A shared_ptr to a chroot facet object.
    typedef std::shared_ptr<chroot_facet_session> ptr;

    /// A shared_ptr to a const chroot facet object.
    typedef std::shared_ptr<const chroot_facet_session> const_ptr;

  private:
    /// The constructor.
    chroot_facet_session ();

  public:
    /// The destructor.
    virtual ~chroot_facet_session ();

    /**
     * Create a chroot facet.
     *
     * @returns a shared_ptr to the new chroot facet.
     */
    static ptr
    create ();

    virtual chroot_facet::ptr
    clone () const;

    virtual std::string const&
    get_name () const;

    /**
     * Get the original name of the chroot (prior to session cloning).
     *
     * @returns the name.
     */
    std::string const&
    get_original_name () const;

    /**
     * Set the original name of the chroot (prior to session cloning).
     * This will also set the selected name.
     *
     * @param name the name.
     */
    void
    set_original_name (std::string const& name);

    /**
     * Get the selected name of the chroot (alias used).
     *
     * @returns the name.
     */
    std::string const&
    get_selected_name () const;

    /**
     * Set the selected name of the chroot (alias used).
     *
     * @param name the name.
     */
    void
    set_selected_name (std::string const& name);

    virtual void
    setup_env (chroot const& chroot,
               environment&  env) const;

    virtual chroot::session_flags
    get_session_flags (chroot const& chroot) const;

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
    /// Original chroot name prior to session cloning.
    std::string  original_chroot_name;
    /// Selected chroot name.
    std::string  selected_chroot_name;
  };

}

#endif /* SBUILD_CHROOT_FACET_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
