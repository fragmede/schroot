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

#include "schroot-main-base.h"

#include <sbuild/sbuild-config.h>
#ifdef SBUILD_FEATURE_PAM
#include <sbuild/sbuild-auth-pam.h>
#include <sbuild/sbuild-auth-pam-conv.h>
#include <sbuild/sbuild-auth-pam-conv-tty.h>
#endif // SBUILD_FEATURE_PAM

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <locale>

#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::endl;
using boost::format;
using sbuild::_;
using sbuild::N_;
using namespace schroot;

namespace
{

  typedef std::pair<main_base::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      // TRANSLATORS: %1% = comma-separated list of chroot names
      emap(main_base::CHROOTS_NOTFOUND,  N_("%1%: Chroots not found")),
      // TRANSLATORS: %4% = file
      emap(main_base::CHROOT_FILE,       N_("No chroots are defined in '%4%'")),
      // TRANSLATORS: %4% = file
      // TRANSLATORS: %5% = file
      emap(main_base::CHROOT_FILE2,      N_("No chroots are defined in '%4%' or '%5%'")),
      // TRANSLATORS: %1% = file
      emap(main_base::CHROOT_NOTDEFINED, N_("The specified chroots are not defined in '%1%'")),
      // TRANSLATORS: %1% = chroot name
      emap(main_base::CHROOT_NOTFOUND,   N_("%1%: Chroot not found")),
      emap(main_base::SESSION_INVALID,   N_("%1%: Invalid session name"))
    };

}

/// Error code to description mapping.
template<>
sbuild::error<main_base::error_code>::map_type
sbuild::error<main_base::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

main_base::main_base (std::string const& program_name,
		      std::string const& program_usage,
		      options_base::ptr& options,
		      bool               use_syslog):
  schroot_base::main(program_name, program_usage,
		     std::tr1::static_pointer_cast<schroot_base::options>(options),
		     use_syslog),
  options(options)
{
}

main_base::~main_base ()
{
}

void
main_base::action_version (std::ostream& stream)
{
  schroot_base::main::action_version(stream);

  format feature("  %1$-12s %2%\n");

  stream << '\n'
	 << _("Available chroot types:") << '\n';
#ifdef SBUILD_FEATURE_BLOCKDEV
  stream << feature % "BLOCKDEV" % _("Support for 'block-device' chroots");
#endif
#ifdef SBUILD_FEATURE_BTRFSSNAP
  stream << feature % "BTRFSSNAP" % _("Support for 'btrfs-snapshot' chroots");
#endif
  stream << feature % "DIRECTORY" % _("Support for 'directory' chroots");
  stream << feature % "FILE" % _("Support for 'file' chroots");
#ifdef SBUILD_FEATURE_LOOPBACK
  stream << feature % "LOOPBACK" % _("Support for 'loopback' chroots");
#endif
#ifdef SBUILD_FEATURE_LVMSNAP
  stream << feature % "LVMSNAP" % _("Support for 'lvm-snapshot' chroots");
#endif
  stream << feature % "PLAIN" % _("Support for 'plain' chroots");
  stream << std::flush;
}

void
main_base::action_info ()
{
  this->config->print_chroot_info(this->chroots, std::cout);
}

void
main_base::action_location ()
{
  this->config->print_chroot_location(this->chroots, std::cout);
}

void
main_base::compat_check ()
{
}

sbuild::string_list
main_base::get_chroot_options ()
{
  sbuild::string_list ret;

  if (this->options->all_chroots == true ||
      this->options->all_sessions == true ||
      this->options->all_source_chroots == true)
    {
      if (this->options->all_chroots)
	{
	  sbuild::string_list chroots;
	  if (this->options->action == options_base::ACTION_LIST)
	    chroots = this->config->get_alias_list("chroot");
	  else
	    chroots = this->config->get_chroot_list("chroot");
	  ret.insert(ret.end(), chroots.begin(), chroots.end());
	}
      if (this->options->all_sessions)
	{
	  sbuild::string_list sessions;
	  if (this->options->action == options_base::ACTION_LIST)
	    sessions = this->config->get_alias_list("session");
	  else
	    sessions = this->config->get_chroot_list("session");
	  ret.insert(ret.end(), sessions.begin(), sessions.end());
	}
      if (this->options->all_source_chroots)
	{
	  sbuild::string_list sources;
	  if (this->options->action == options_base::ACTION_LIST)
	    sources = this->config->get_alias_list("source");
	  else
	    sources = this->config->get_chroot_list("source");
	  ret.insert(ret.end(), sources.begin(), sources.end());
	}
    }
  else
    {
      sbuild::string_list invalid_chroots =
	this->config->validate_chroots(this->options->chroots);

      if (!invalid_chroots.empty())
	{
	  std::string invalid_list;
	  for (sbuild::string_list::const_iterator chroot =
		 invalid_chroots.begin();
	       chroot != invalid_chroots.end();
	       ++chroot)
	    {
	      invalid_list += *chroot;
	      if (chroot + 1 != invalid_chroots.end())
		invalid_list += ", ";
	    }
	  throw error(invalid_list,
		      (invalid_chroots.size() == 1)
		      ? CHROOT_NOTFOUND : CHROOTS_NOTFOUND);
	}
      ret = this->options->chroots;
    }

  return ret;
}

void
main_base::load_config ()
{
  this->config = sbuild::chroot_config::ptr(new sbuild::chroot_config);
  /* The normal chroot list is used when starting a session or running
     any chroot type or session, or displaying chroot information. */
  if (this->options->load_chroots == true)
    {
      this->config->add("chroot", SCHROOT_CONF);
      this->config->add("chroot", SCHROOT_CONF_CHROOT_D);
    }
  /* The session chroot list is used when running or ending an
     existing session, or displaying chroot information. */
  if (this->options->load_sessions == true)
    this->config->add("session", SCHROOT_SESSION_DIR);
}

int
main_base::run_impl ()
{
  compat_check();

  if (this->options->action == options_base::ACTION_HELP)
    {
      action_help(std::cout);
      return EXIT_SUCCESS;
    }

  if (this->options->action == options_base::ACTION_VERSION)
    {
      action_version(std::cout);
      return EXIT_SUCCESS;
    }

  /* Initialise chroot configuration. */
  load_config();

  if (this->options->load_chroots &&
      this->config->get_chroots("chroot").empty() &&
      this->options->quiet == false)
    log_exception_warning(error(CHROOT_FILE2, SCHROOT_CONF, SCHROOT_CONF_CHROOT_D));

  /* Get list of chroots to use */
  chroots = get_chroot_options();
  if (this->chroots.empty())
    {
      if (!(this->options->all_chroots == true ||
	    this->options->all_sessions == true))
	throw error(SCHROOT_CONF, CHROOT_NOTDEFINED);
      else
	{
	  // If one of the --all options was used, then don't treat
	  // the lack of chroots as an error.  TODO: Also check if any
	  // additional chroots were specified with -c; this needs
	  // changes in get_chroot_options.
	  log_exception_warning(error((this->options->all_chroots == true) ?
				      SCHROOT_CONF : SCHROOT_SESSION_DIR,
				      CHROOT_NOTDEFINED));
	  return EXIT_SUCCESS;
	}
    }

  /* Print chroot list (including aliases). */
  if (this->options->action == options_base::ACTION_LIST)
    {
      action_list();
      return EXIT_SUCCESS;
    }

  if (this->config->find_alias("session", this->options->session_name))
    throw error(this->options->session_name, SESSION_INVALID);

  /* Print chroot information for specified chroots. */
  if (this->options->action == options_base::ACTION_INFO)
    {
      action_info();
      return EXIT_SUCCESS;
    }
  if (this->options->action == options_base::ACTION_LOCATION)
    {
      action_location();
      return EXIT_SUCCESS;
    }
  if (this->options->action == options_base::ACTION_CONFIG)
    {
      action_config();
      return EXIT_SUCCESS;
    }

  /* Create a session. */
  sbuild::session::operation sess_op(sbuild::session::OPERATION_AUTOMATIC);
  if (this->options->action == options_base::ACTION_SESSION_BEGIN)
    sess_op = sbuild::session::OPERATION_BEGIN;
  else if (this->options->action == options_base::ACTION_SESSION_RECOVER)
    sess_op = sbuild::session::OPERATION_RECOVER;
  else if (this->options->action == options_base::ACTION_SESSION_RUN)
    sess_op = sbuild::session::OPERATION_RUN;
  else if (this->options->action == options_base::ACTION_SESSION_END)
    sess_op = sbuild::session::OPERATION_END;

  try
    {
      create_session(sess_op);
      add_session_auth();

      if (!this->options->command.empty())
	this->session->get_auth()->set_command(this->options->command);
      if (!this->options->directory.empty())
	this->session->get_auth()->set_wd(this->options->directory);
      this->session->set_preserve_environment(this->options->preserve);
      this->session->set_session_id(this->options->session_name);
      this->session->set_force(this->options->session_force);
      if (this->options->quiet)
	this->session->set_verbosity("quiet");
      else if (this->options->verbose)
	this->session->set_verbosity("verbose");

      /* Run session. */
      this->session->run();
    }
  catch (std::runtime_error const& e)
    {
      if (!this->options->quiet)
	sbuild::log_exception_error(e);
    }

  if (this->session)
    return this->session->get_child_status();
  else
    return EXIT_FAILURE;
}

void
main_base::add_session_auth ()
{
  // Add PAM authentication handler.  If PAM isn't available, just
  // continue to use the default handler

#ifdef SBUILD_FEATURE_PAM
  sbuild::auth::ptr auth = sbuild::auth_pam::create("schroot");

  sbuild::auth_pam_conv::auth_ptr pam_auth =
    std::tr1::dynamic_pointer_cast<sbuild::auth_pam>(auth);

  sbuild::auth_pam_conv::ptr conv = sbuild::auth_pam_conv_tty::create(pam_auth);


  /* Set up authentication timeouts. */
  time_t curtime = 0;
  time(&curtime);
  conv->set_warning_timeout(curtime + 15);
  conv->set_fatal_timeout(curtime + 20);
  pam_auth->set_conv(conv);

  this->session->set_auth(auth);
#endif // SBUILD_FEATURE_PAM
}
