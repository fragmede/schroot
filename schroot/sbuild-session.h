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
#include <tr1/memory>

#include <sys/types.h>
#include <sys/wait.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "sbuild-auth.h"
#include "sbuild-config.h"

typedef enum
{
  SBUILD_SESSION_ERROR_FORK,
  SBUILD_SESSION_ERROR_CHILD,
  SBUILD_SESSION_ERROR_CHROOT,
  SBUILD_SESSION_ERROR_CHROOT_SETUP
} SbuildSessionError;

#define SBUILD_SESSION_ERROR sbuild_session_error_quark()

typedef enum
{
  SBUILD_SESSION_OPERATION_AUTOMATIC,
  SBUILD_SESSION_OPERATION_BEGIN,
  SBUILD_SESSION_OPERATION_RECOVER,
  SBUILD_SESSION_OPERATION_END,
  SBUILD_SESSION_OPERATION_RUN
} SbuildSessionOperation;

GQuark
sbuild_session_error_quark (void);

class SbuildSession : public SbuildAuth
{
public:
  SbuildSession (const std::string&                  service,
		 std::tr1::shared_ptr<SbuildConfig>& config,
		 SbuildSessionOperation              operation,
		 string_list                         chroots);
  virtual ~SbuildSession();

  std::tr1::shared_ptr<SbuildConfig>&
  get_config ();

  void
  set_config (std::tr1::shared_ptr<SbuildConfig>&);

  const string_list&
  get_chroots () const;

  void
  set_chroots (const string_list& chroots);

  SbuildSessionOperation
  get_operation () const;

  void
  set_operation (SbuildSessionOperation  operation);

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

  virtual SbuildAuthStatus
  get_auth_status () const;

  virtual bool
  run_impl (GError **error);

private:
  int
  exec (const std::string& file,
	const string_list& command,
	const env_list& env);


  bool
  setup_chroot (SbuildChroot&           session_chroot,
		SbuildChrootSetupType   setup_type,
		GError                **error);

  bool
  run_chroot (SbuildChroot&   session_chroot,
	      GError        **error);

  void
  run_child (SbuildChroot& session_chroot);

  bool
  wait_for_child (int      pid,
		  GError **error);

  std::tr1::shared_ptr<SbuildConfig> config;
  string_list                        chroots;
  int                                child_status;
  SbuildSessionOperation             operation;
  std::string                        session_id;
  bool                               force;
};

#endif /* SBUILD_SESSION_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
