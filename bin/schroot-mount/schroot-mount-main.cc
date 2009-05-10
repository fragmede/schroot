/* Copyright © 2005-2007  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include <sbuild/sbuild-mntstream.h>

#include "schroot-mount-main.h"

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include <mntent.h>

using std::endl;
using boost::format;
using sbuild::_;
using sbuild::N_;
using namespace schroot_mount;

namespace
{

  typedef std::pair<main::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(main::CHILD_FORK, N_("Failed to fork child")),
      emap(main::CHILD_WAIT, N_("Wait for child failed")),
      // TRANSLATORS: %1% = command name
      emap(main::EXEC,       N_("Failed to execute '%1%'"))
    };

}

template<>
sbuild::error<main::error_code>::map_type
sbuild::error<main::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

main::main (options::ptr& options):
  schroot_base::main("schroot-mount",
		     // TRANSLATORS: '...' is an ellipsis e.g. U+2026,
		     // and '-' is an em-dash.
		     _("[OPTION...] - mount filesystems"),
		     options,
		     false),
  opts(options)
{
}

main::~main ()
{
}

void
main::action_mount ()
{
  // Check mounts.
  sbuild::mntstream mounts(opts->fstab);

  sbuild::mntstream::mntentry entry;

  while (mounts >> entry)
    {
      // Ensure entry has a leading / to prevent security hole where
      // mountpoint might be outside the chroot.
      std::string d = entry.directory;
      if (d.empty() || d[0] != '/')
	d = std::string("/") + d;

      std::string directory(opts->mountpoint + entry.directory);

      if (!boost::filesystem::is_directory(directory))
	{
	  sbuild::log_debug(sbuild::DEBUG_INFO)
	    << boost::format("Creating '%1%' in '%2%'")
	    % entry.directory
	    % opts->mountpoint
	    << std::endl;

	  if (!opts->dry_run)
	    {
	      try
	        {
	          boost::filesystem::create_directories(directory);
	        }
	      catch (std::exception const& e)
	        {
	          sbuild::log_exception_error(e);
	          exit(EXIT_FAILURE);
	        }
	      catch (...)
	        {
	          sbuild::log_error()
	            << _("An unknown exception occurred") << std::endl;
	          exit(EXIT_FAILURE);
	        }
	    }
        }

      sbuild::log_debug(sbuild::DEBUG_INFO)
      	<< boost::format("Mounting '%1%' on '%2%'")
	% entry.filesystem_name
	% directory
	<< std::endl;

      if (!opts->dry_run)
	{
	  sbuild::string_list command;
	  command.push_back("/bin/mount");
	  if (opts->verbose)
	    command.push_back("-v");
	  command.push_back("-t");
	  command.push_back(entry.type);
	  command.push_back("-o");
	  command.push_back(entry.options);
	  command.push_back(entry.filesystem_name);
	  command.push_back(directory);

	  int status = run_child(command[0], command, sbuild::environment());

	  if (status)
	    exit(status);
	}
    }
}

int
main::run_child (std::string const& file,
		 sbuild::string_list const& command,
		 sbuild::environment const& env)
{
  int exit_status = 0;
  pid_t pid;

  if ((pid = fork()) == -1)
    {
      throw error(CHILD_FORK, strerror(errno));
    }
  else if (pid == 0)
    {
      try
	{
	  sbuild::log_debug(sbuild::DEBUG_INFO)
	    << "mount_main: executing "
	    << sbuild::string_list_to_string(command, ", ")
	    << std::endl;
	  exec(file, command, env);
	  error e(file, EXEC, strerror(errno));
	  sbuild::log_exception_error(e);
	}
      catch (std::exception const& e)
	{
	  sbuild::log_exception_error(e);
	}
      catch (...)
	{
	  sbuild::log_error()
	    << _("An unknown exception occurred") << std::endl;
	}
      _exit(EXIT_FAILURE);
    }
  else
    {
      wait_for_child(pid, exit_status);
    }

  if (exit_status)
    sbuild::log_debug(sbuild::DEBUG_INFO)
      << "mount_main: " << file
      << " failed with status " << exit_status
      << std::endl;
  else
    sbuild::log_debug(sbuild::DEBUG_INFO)
      << "mount_main: " << file
      << " succeeded"
      << std::endl;

  return exit_status;
}

void
main::wait_for_child (pid_t pid,
		      int&  child_status)
{
  child_status = EXIT_FAILURE; // Default exit status

  int status;

  while (1)
    {
      if (waitpid(pid, &status, 0) == -1)
	{
	  if (errno == EINTR)
	    continue; // Wait again.
	  else
	    throw error(CHILD_WAIT, strerror(errno));
	}
      else
	break;
    }

  if (WIFEXITED(status))
    child_status = WEXITSTATUS(status);
}

int
main::run_impl ()
{
  if (this->opts->action == options::ACTION_HELP)
    action_help(std::cerr);
  else if (this->opts->action == options::ACTION_VERSION)
    action_version(std::cerr);
  else if (this->opts->action == options::ACTION_MOUNT)
    action_mount();
  else
    assert(0); // Invalid action.

  return EXIT_SUCCESS;
}
