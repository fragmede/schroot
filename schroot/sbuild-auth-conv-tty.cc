/* sbuild-auth-conv-tty - sbuild auth terminal conversation object
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

/**
 * SECTION:sbuild-auth-conv-tty
 * @short_description: authentication terminal conversation object
 * @title: SbuildAuthConvTty
 *
 * This class is an implementation of the #SbuildAuthConv interface,
 * and is used to interact with the user on a terminal (TTY)
 * interface.
 *
 * In order to implement timeouts, this class uses alarm(2).  This has
 * some important implications.  Global state is modified by the
 * object, so only one may be used at once in a single process.  In
 * addition, no other part of the process may set or unset the SIGALRM
 * handlers and the alarm(2) timer during the time PAM authentication
 * is proceeding.
 */

#include <config.h>

#include <iostream>

#include <termios.h>
#include <unistd.h>

#include "sbuild-i18n.h"
#include "sbuild-auth-conv-tty.h"
#include "sbuild-error.h"
#include "sbuild-util.h"

using std::cerr;
using std::endl;
using namespace sbuild;

AuthConvTty::AuthConvTty():
  warning_timeout(0),
  fatal_timeout(0),
  start_time(0)
{
}

AuthConvTty::~AuthConvTty()
{
}

/**
 * get_warning_timeout:
 * @conv_tty: an #AuthConvTty.
 *
 * Get the warning timeout for @conv_tty.
 *
 * Returns the timeout.
 */
time_t
AuthConvTty::get_warning_timeout ()
{
  return this->warning_timeout;
}

/**
 * set_warning_timeout:
 * @conv_tty: an #AuthConvTty.
 * @timeout: the timeout to set.
 *
 * Set the warning timeout for @conv_tty.
 */
void
AuthConvTty::set_warning_timeout (time_t timeout)
{
  this->warning_timeout = timeout;
}

/**
 * get_fatal_timeout:
 * @conv_tty: an #AuthConvTty.
 *
 * Get the fatal timeout for @conv_tty.
 *
 * Returns the timeout.
 */
time_t
AuthConvTty::get_fatal_timeout ()
{
  return this->fatal_timeout;
}

/**
 * set_fatal_timeout:
 * @conv_tty: an #AuthConvTty.
 * @timeout: the timeout to set.
 *
 * Set the fatal timeout for @conv_tty.
 */
void
AuthConvTty::set_fatal_timeout (time_t timeout)
{
  this->fatal_timeout = timeout;
}

static volatile sig_atomic_t timer_expired = false;

/**
 * reset_alarm:
 * @conv_tty: an #AuthConvTty.
 *
 * Cancel any alarm set previously, and restore the state of SIGALRM
 * prior to the conversation.
 */
static void
reset_alarm (struct sigaction *orig_sa)
{
  /* Stop alarm */
  alarm (0);
 /* Restore original handler */
  sigaction (SIGALRM, orig_sa, NULL);
}

/**
 * alarm_handler:
 * @ignore: the signal number.
 *
 * Handle the SIGALRM signal.
 */
static void
alarm_handler (int ignore)
{
  timer_expired = TRUE;
}

/**
 * set_alarm:
 * @conv_tty: an #AuthConvTty.
 * @orig_sa: the original signal handler
 *
 * Set the SIGALARM handler, and set the timeout to @delay seconds.
 * The old signal handler is stored in @orig_sa.
 */
static bool
set_alarm (int delay,
	   struct sigaction *orig_sa)
{
  struct sigaction new_sa;
  sigemptyset(&new_sa.sa_mask);
  new_sa.sa_flags = 0;
  new_sa.sa_handler = alarm_handler;

  if (sigaction(SIGALRM, &new_sa, orig_sa) != 0)
    {
      return FALSE;
    }
  if (alarm(delay) != 0)
    {
      sigaction(SIGALRM, orig_sa, NULL);
      return FALSE;
    }

  return TRUE;
}

/**
 * get_delay:
 * @conv_tty: an #AuthConvTty.
 *
 * Get the time delay before the next SIGALRM signal.  If either the
 * warning timeout or the fatal timeout have expired, a message to
 * notify the user is printed to stderr.
 *
 * Returns the delay in seconds, 0 if no delay is set, or -1 if the
 * fatal timeout has expired.
 */
int
AuthConvTty::get_delay ()
{
  timer_expired = 0;
  time (&this->start_time);

  if (this->fatal_timeout != 0 &&
      this->start_time >= this->fatal_timeout)
    {
      cerr << _("Timed out") << endl;
      return -1;
    }

  if (this->warning_timeout != 0 &&
      this->start_time >= this->warning_timeout)
    {
      cerr << _("Time is running out...") << endl;
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

/**
 * read_string:
 * @conv_tty: an #AuthConvTty.
 * @message: the message to prompt the user for input.
 * @echo: echo user input to screen
 *
 * Read user input from standard input.  The prompt @message is
 * printed to prompt the user for input.  If @echo is TRUE, the user
 * input it echoed back to the terminal, but if FALSE, echoing is
 * suppressed using termios(3).
 *
 * If the SIGALRM timer expires while waiting for input, this is
 * handled by re-checking the delay time which will warn the user or
 * cause the input routine to terminate if the fatal timeout has
 * expired.
 *
 * Returns a string, or NULL on failure.  The string must be freed by
 * the caller.
 */
std::string *
AuthConvTty::read_string (std::string message,
			  bool        echo)
{
  struct termios orig_termios, noecho_termios;
  struct sigaction saved_signals;
  sigset_t old_sigs, new_sigs;
  bool use_termios = false;
  std::string *return_input = 0;

  if (isatty(STDIN_FILENO))
    {
      use_termios = TRUE;

      if (tcgetattr(STDIN_FILENO, &orig_termios) != 0)
	{
	  g_error("Failed to get terminal settings");
	  return NULL;
	}

      memcpy(&noecho_termios, &orig_termios, sizeof(struct termios));

      if (echo == FALSE)
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
      cerr << message << endl;

      if (use_termios == TRUE)
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &noecho_termios);

      if (delay > 0 && set_alarm(delay, &saved_signals) == FALSE)
	break;
      else
	{
	  int nchars = read(STDIN_FILENO, input, PAM_MAX_MSG_SIZE - 1);
	  if (use_termios)
	    {
	      tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
	      if (echo == FALSE && timer_expired == TRUE)
		cerr << endl;
	    }
	  if (delay > 0)
	    reset_alarm(&saved_signals);
	  if (timer_expired == TRUE)
	    {
	      delay = get_delay();
	    }
	  else if (nchars > 0)
	    {
	      if (echo == FALSE)
		cerr << endl;

	      if (input[nchars-1] == '\n')
		input[--nchars] = '\0';
	      else
		input[nchars] = '\0';

	      return_input = new std::string(input);
	      break;
	    }
	  else if (nchars == 0)
	    {
	      if (echo == FALSE)
		cerr << endl;

	      return_input = new std::string();
	      break;
	    }
	}
    }

  memset(input, 0, sizeof(input));

  if (use_termios == TRUE)
    {
      sigprocmask(SIG_SETMASK, &old_sigs, NULL);
      tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
    }

  return return_input;
}

/**
 * conversation:
 * @conv_tty: an #AuthConvTty.
 * @num_messages: the number of messages
 * @messages: the messages to display to the user
 *
 * Hold a conversation with the user.
 *
 * Returns TRUE on success, FALSE on failure.
 */
bool
AuthConvTty::conversation_impl (std::vector<AuthMessage>& messages)
{
  for (std::vector<AuthMessage>::iterator cur = messages.begin();
       cur != messages.end();
       ++cur)
    {
      std::string *str = 0;

      switch (cur->type)
	{
	case AuthMessage::MESSAGE_PROMPT_NOECHO:
	  str = read_string(cur->message, FALSE);
	  if (str == 0)
	    return FALSE;
	  cur->response = *str;
	  delete str;
	  str = 0;
	  break;
	case AuthMessage::MESSAGE_PROMPT_ECHO:
	  str = read_string(cur->message, TRUE);
	  if (str == 0)
	    return FALSE;
	  cur->response = *str;
	  delete str;
	  str = 0;
	  break;
	case AuthMessage::MESSAGE_ERROR:
	  cerr << cur->message << endl;
	  break;
	case AuthMessage::MESSAGE_INFO:
	  cerr << cur->message << endl;
	  break;
	default:
	  cerr << format_string(_("Unsupported conversation type %d"),
				cur->type)
	       << endl;
	  return FALSE;
	  break;
	}
    }

  return TRUE;
}

/*
 * Local Variables:
 * mode:C++
 * End:
 */
