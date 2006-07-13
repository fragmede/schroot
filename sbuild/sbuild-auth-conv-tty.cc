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

#include "sbuild-auth-conv-tty.h"
#include "sbuild-log.h"

#include <iostream>

#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include <boost/format.hpp>

using std::cerr;
using std::endl;
using boost::format;
using namespace sbuild;

namespace
{

  typedef std::pair<auth_conv_tty::error_code,const char *> emap;

  /**
   * This is a list of the supported error codes.  It's used to
   * construct the real error codes map.
   */
  emap init_errors[] =
    {
      emap(auth_conv_tty::TIMEOUT,         N_("Timed out")),
      emap(auth_conv_tty::TIMEOUT_PENDING, N_("Time is running out...")),
      emap(auth_conv_tty::TERMIOS,         N_("Failed to get terminal settings")),
      emap(auth_conv_tty::CONV_TYPE,       N_("Unsupported conversation type"))
    };

  volatile sig_atomic_t timer_expired = false;

  /**
   * Disable the alarm and signal handler.
   *
   * @param orig_sa the signal handler to restore.
   */
  void
  reset_alarm (struct sigaction *orig_sa)
  {
    // Stop alarm
    alarm (0);
    // Restore original handler
    sigaction (SIGALRM, orig_sa, NULL);
  }

  /**
   * Handle the SIGALRM signal.
   *
   * @param ignore the signal number (unused).
   */
  void
  alarm_handler (int ignore)
  {
    timer_expired = true;
  }

  /**
   * Set the SIGALARM handler, and set the timeout to delay seconds.
   * The old signal handler is stored in orig_sa.
   *
   * @param delay the delay (in seconds) before SIGALRM is raised.
   * @param orig_sa the location to store the original signal handler.
   * @returns true on success, false on failure.
   */
  bool
  set_alarm (int delay,
	     struct sigaction *orig_sa)
  {
    struct sigaction new_sa;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    new_sa.sa_handler = alarm_handler;

    if (sigaction(SIGALRM, &new_sa, orig_sa) != 0)
      {
	return false;
      }
    if (alarm(delay) != 0)
      {
	sigaction(SIGALRM, orig_sa, NULL);
	return false;
      }

    return true;
  }

}

template<>
error<auth_conv_tty::error_code>::map_type
error<auth_conv_tty::error_code>::error_strings
(init_errors,
 init_errors + (sizeof(init_errors) / sizeof(init_errors[0])));

auth_conv_tty::auth_conv_tty ():
  warning_timeout(0),
  fatal_timeout(0),
  start_time(0)
{
}

auth_conv_tty::~auth_conv_tty ()
{
}

time_t
auth_conv_tty::get_warning_timeout ()
{
  return this->warning_timeout;
}

void
auth_conv_tty::set_warning_timeout (time_t timeout)
{
  this->warning_timeout = timeout;
}

time_t
auth_conv_tty::get_fatal_timeout ()
{
  return this->fatal_timeout;
}

void
auth_conv_tty::set_fatal_timeout (time_t timeout)
{
  this->fatal_timeout = timeout;
}

int
auth_conv_tty::get_delay ()
{
  timer_expired = 0;
  time (&this->start_time);

  if (this->fatal_timeout != 0 &&
      this->start_time >= this->fatal_timeout)
    throw error(TIMEOUT);

  if (this->warning_timeout != 0 &&
      this->start_time >= this->warning_timeout)
    {
      error e(TIMEOUT_PENDING);
      log_warning() << e.what() << endl;
      return (this->fatal_timeout ?
	      this->fatal_timeout - this->start_time : 0);
    }

  if (this->warning_timeout != 0)
    return this->warning_timeout - this->start_time;
  else if (this->fatal_timeout != 0)
    return this->fatal_timeout - this->start_time;
  else
    return 0;
}

std::string
auth_conv_tty::read_string (std::string message,
			    bool        echo)
{
  struct termios orig_termios, noecho_termios;
  struct sigaction saved_signals;
  sigset_t old_sigs, new_sigs;
  bool use_termios = false;
  std::string retval;

  if (isatty(STDIN_FILENO))
    {
      use_termios = true;

      if (tcgetattr(STDIN_FILENO, &orig_termios) != 0)
	throw error(TERMIOS);

      memcpy(&noecho_termios, &orig_termios, sizeof(struct termios));

      if (echo == false)
	noecho_termios.c_lflag &= ~(ECHO);

      sigemptyset(&new_sigs);
      sigaddset(&new_sigs, SIGINT);
      sigaddset(&new_sigs, SIGTSTP);
      sigprocmask(SIG_BLOCK, &new_sigs, &old_sigs);
    }

  char input[PAM_MAX_MSG_SIZE];

  int delay = get_delay();

  while (delay >= 0)
    {
      cerr << message;

      if (use_termios == true)
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &noecho_termios);

      if (delay > 0 && set_alarm(delay, &saved_signals) == false)
	break;
      else
	{
	  int nchars = read(STDIN_FILENO, input, PAM_MAX_MSG_SIZE - 1);
	  if (use_termios)
	    {
	      tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
	      if (echo == false && timer_expired == true)
		cerr << endl;
	    }
	  if (delay > 0)
	    reset_alarm(&saved_signals);
	  if (timer_expired == true)
	    {
	      delay = get_delay();
	    }
	  else if (nchars > 0)
	    {
	      if (echo == false)
		cerr << endl;

	      if (input[nchars-1] == '\n')
		input[--nchars] = '\0';
	      else
		input[nchars] = '\0';

	      retval = input;
	      break;
	    }
	  else if (nchars == 0)
	    {
	      if (echo == false)
		cerr << endl;

	      retval = "";
	      break;
	    }
	}
    }

  memset(input, 0, sizeof(input));

  if (use_termios == true)
    {
      sigprocmask(SIG_SETMASK, &old_sigs, NULL);
      tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
    }

  return retval;
}

bool
auth_conv_tty::conversation (message_list& messages)
{
  try
    {
      for (std::vector<auth_message>::iterator cur = messages.begin();
	   cur != messages.end();
	   ++cur)
	{
	  switch (cur->type)
	    {
	    case auth_message::MESSAGE_PROMPT_NOECHO:
	      cur->response = read_string(cur->message, false);
	      break;
	    case auth_message::MESSAGE_PROMPT_ECHO:
	      cur->response = read_string(cur->message, true);
	      break;
	    case auth_message::MESSAGE_ERROR:
	      cerr << cur->message << endl;
	      break;
	    case auth_message::MESSAGE_INFO:
	      cerr << cur->message << endl;
	      break;
	    default:
	      {
		format fmt("%1%");
		fmt % cur->type;
		error e(fmt.str(), CONV_TYPE);
		log_error() << e.what() << endl;
		return false;
	      }
	      break;
	    }
	}
    }
  catch (error const& e)
    {
      log_error() << e.what() << std::endl;
      return false;
    }

  return true;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
