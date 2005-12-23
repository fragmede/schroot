/* sbuild-session - sbuild session object
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

#ifndef SBUILD_SESSION_H
#define SBUILD_SESSION_H

#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "sbuild-auth.h"
#include "sbuild-config.h"
#include "sbuild-error.h"

namespace sbuild
{

  class Session : public Auth
  {
  public:
    typedef enum
      {
	OPERATION_AUTOMATIC,
	OPERATION_BEGIN,
	OPERATION_RECOVER,
	OPERATION_END,
	OPERATION_RUN
      } Operation;

    typedef enum
      {
	ERROR_FORK,
	ERROR_CHILD,
	ERROR_CHROOT,
	ERROR_CHROOT_SETUP
      } ErrorCode;

    typedef Exception<ErrorCode> error;

    Session (const std::string&            service,
	     std::tr1::shared_ptr<Config>& config,
	     Operation                     operation,
	     string_list                   chroots);
    virtual ~Session();

    std::tr1::shared_ptr<Config>&
    get_config ();

    void
    set_config (std::tr1::shared_ptr<Config>&);

    const string_list&
    get_chroots () const;

    void
    set_chroots (const string_list& chroots);

    Operation
    get_operation () const;

    void
    set_operation (Operation operation);

    const std::string&
    get_session_id () const;

    void
    set_session_id (const std::string& session_id);

    bool
    get_force () const;

    void
    set_force (bool force);

    int
    get_child_status () const;

    virtual sbuild::Auth::Status
    get_auth_status () const;

    virtual void
    run_impl ();

  private:
    int
    exec (const std::string& file,
	  const string_list& command,
	  const env_list& env);

    void
    setup_chroot (Chroot&           session_chroot,
		  Chroot::SetupType setup_type);

    void
    run_chroot (Chroot& session_chroot);

    void
    run_child (Chroot& session_chroot);

    void
    wait_for_child (int  pid,
		    int& child_status);

    std::tr1::shared_ptr<Config> config;
    string_list                  chroots;
    int                          child_status;
    Operation                    operation;
    std::string                  session_id;
    bool                         force;
  };

}

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
