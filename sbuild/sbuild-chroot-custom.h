/* Copyright © 2005-2012  Roger Leigh <rleigh@debian.org>
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

#ifndef SBUILD_CHROOT_CUSTOM_H
#define SBUILD_CHROOT_CUSTOM_H

#include <sbuild/sbuild-chroot.h>

namespace sbuild
{

  /**
   * A chroot stored with custom parameters.
   *
   * This chroot specifies no behaviour or policy.  It is entirely
   * configured using user options and setup scripts.  The intent is
   * to permit the prototyping of and experimentation with new chroot
   * types without requiring a "full" class definition and associated
   * infrastructural work.  It also makes schroot extensible without
   * requiring any C++ coding.
   */
  class chroot_custom : public chroot
  {
  protected:
    /// The constructor.
    chroot_custom ();

    /// The copy constructor.
    chroot_custom (const chroot_custom& rhs);

    friend class chroot;

  public:
    /// The destructor.
    virtual ~chroot_custom ();

    virtual chroot::ptr
    clone () const;

    virtual chroot::ptr
    clone_session (std::string const& session_id,
                   std::string const& alias,
                   std::string const& user,
                   bool               root) const;

    virtual chroot::ptr
    clone_source () const;

    /**
     * Enable or disable cloning of the chroot session.  This simply
     * creates or removes the chroot_facet_session_cloneable facet,
     * hence there is no companion get method.
     *
     * @param cloneable true if cloneable, false if not.
     */
    void
    set_session_cloneable (bool cloneable);

    /**
     * Enable or disable purging of the chroot session.  Note that
     * this is only usable if the chroot supports session cloning,
     * otherwise this does nothing.
     *
     * @param purgeable true if purgeable, false if not.
     */
    void
    set_session_purgeable (bool purgeable);

    /**
     * Get status of chroot session purging.
     *
     * @returns true if purgeable, false if not.
     */
    bool
    get_session_purgeable () const;

    /**
     * Enable or disable cloning of the source chroot.  This simply
     * creates or removes the chroot_facet_source_cloneable facet,
     * hence there is no companion get method.
     *
     * @param cloneable true if source cloneable, false if not.
     */
    void
    set_source_cloneable (bool cloneable);

    virtual void
    setup_env (chroot const& chroot,
               environment& env) const;

    virtual std::string const&
    get_chroot_type () const;

    std::string
    get_path () const;

    virtual session_flags
    get_session_flags (chroot const& chroot) const;

  protected:
    virtual void
    setup_lock (chroot::setup_type type,
                bool               lock,
                int                status);

    virtual void
    get_details (chroot const& chroot,
                 format_detail& detail) const;

    virtual void
    get_keyfile (chroot const& chroot,
                 keyfile& keyfile) const;

    virtual void
    set_keyfile (chroot&        chroot,
                 keyfile const& keyfile,
                 string_list&   used_keys);

  private:
    bool purgeable;
  };

}

#endif /* SBUILD_CHROOT_CUSTOM_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
