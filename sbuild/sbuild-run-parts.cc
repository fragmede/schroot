/* Copyright © 2005-2006  Roger Leigh <rleigh@debian.org>
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

#include <config.h>

#include "sbuild-dirstream.h"
#include "sbuild-run-parts.h"
#include "sbuild-util.h"

#include <cerrno>

#include <sys/wait.h>

#include <boost/format.hpp>
#include <boost/regex.hpp>

using boost::format;
using boost::regex;
using namespace sbuild;

namespace
{

  typedef std::pair<sbuild::run_parts::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(run_parts::CHILD_FORK, N_("Failed to fork child")),
      emap(run_parts::CHILD_WAIT, N_("Wait for child failed")),
      // TRANSLATORS: %1% = command name
      emap(run_parts::EXEC,       N_("Failed to execute '%1%'"))
    };

}

template<>
error<run_parts::error_code>::map_type
error<run_parts::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

run_parts::run_parts (std::string const& directory,
		      bool               lsb_mode,
		      bool               abort_on_error,
		      mode_t             umask):
  lsb_mode(true),
  abort_on_error(abort_on_error),
  umask(umask),
  verbose(false),
  reverse(false),
  //  restricted(true),
  directory(directory),
  programs()
{
  dirstream stream(directory);
  direntry de;
  while (stream >> de)
    {
      std::string name(de.name());
      if (check_filename(name))
	this->programs.insert(name);
    }
}

run_parts::~run_parts ()
{
}

bool
run_parts::get_verbose () const
{
  return this->verbose;
}

void
run_parts::set_verbose (bool verbose)
{
  this->verbose = verbose;
}

bool
run_parts::get_reverse () const
{
  return this->reverse;
}

void
run_parts::set_reverse (bool reverse)
{
  this->reverse = reverse;
}

int
run_parts::run (string_list const& command,
		environment const& env)
{
  int exit_status = 0;

  if (!this->reverse)
    {
      for (program_set::const_iterator pos = this->programs.begin();
	   pos != this->programs.end();
	   ++pos)
	{
	  string_list real_command;
	  real_command.push_back(*pos);
	  for (string_list::const_iterator spos = command.begin();
	       spos != command.end();
	       ++spos)
	    real_command.push_back(*spos);

	  exit_status = run_child(*pos, real_command, env);

	  if (exit_status && this->abort_on_error)
	    return exit_status;
	}
    }
  else
    {
      for (program_set::const_reverse_iterator pos = this->programs.rbegin();
	   pos != this->programs.rend();
	   ++pos)
	{
	  string_list real_command;
	  real_command.push_back(*pos);
	  for (string_list::const_iterator spos = command.begin();
	       spos != command.end();
	       ++spos)
	    real_command.push_back(*spos);

	  exit_status = run_child(*pos, real_command, env);

	  if (exit_status && this->abort_on_error)
	    return exit_status;
	}
    }

  return exit_status;
}

int
run_parts::run_child (std::string const& file,
		      string_list const& command,
		      environment const& env)
{
  int exit_status = 0;
  pid_t pid;

  if ((pid = fork()) == -1)
    {
      throw error(CHILD_FORK, strerror(errno));
    }
  else if (pid == 0)
    {
      log_debug(DEBUG_INFO) << "run_parts: executing "
			    << string_list_to_string(command, ", ")
			    << std::endl;
      if (this->verbose)
	// TRANSLATORS: %1% = command
	log_info() << format(_("Executing '%1%'"))
	  % string_list_to_string(command, " ")
		   << std::endl;
      ::umask(this->umask);
      exec(this->directory + '/' + file, command, env);
      error e(file, EXEC, strerror(errno));
      log_exception_error(e);
      _exit(EXIT_FAILURE);
    }
  else
    {
      wait_for_child(pid, exit_status);
    }

  return exit_status;
}

void
run_parts::wait_for_child (pid_t pid,
			   int&  child_status)
{
  child_status = EXIT_FAILURE; // Default exit status

  int status;

  while (1)
    {
      if (wait(&status) != pid)
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

bool
run_parts::check_filename (std::string const& name)
{
  bool match = false;

  if (this->lsb_mode)
    {
      static regex lanana_namespace("^[a-z0-9]+$", boost::regex::basic);
      static regex lsb_namespace("^_?([a-z0-9_.]+-)+[a-z0-9]+$",
				 boost::regex::basic);
      static regex debian_cron_namespace("^[a-z0-9][a-z0-9-]*$",
					 boost::regex::basic);
      static regex debian_dpkg_conffile_cruft("dpkg-(old|dist|new|tmp)$",
					      boost::regex::extended);

      if ((regex_match(name, lanana_namespace) ||
	   regex_match(name, lsb_namespace) ||
	   regex_match(name, debian_cron_namespace)) &&
	  !regex_match(name, debian_dpkg_conffile_cruft))
	match = true;
    }
  else
    {
      static regex traditional_namespace("^[a-zA-Z0-9_-]$",
					 boost::regex::basic);
      if (regex_match(name, traditional_namespace))
	match = true;
    }

  return match;
}
