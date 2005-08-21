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

#define _GNU_SOURCE

#include <termios.h>
#include <unistd.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

#include "sbuild-auth-conv-tty.h"
#include "sbuild-error.h"
#include "sbuild-marshallers.h"
#include "sbuild-typebuiltins.h"

static time_t
sbuild_auth_conv_tty_get_warning_timeout (const SbuildAuthConvTty *restrict conv_tty);
static void
sbuild_auth_conv_tty_set_warning_timeout (SbuildAuthConvTty *conv_tty,
					  time_t             timeout);
static time_t
sbuild_auth_conv_tty_get_fatal_timeout (const SbuildAuthConvTty *restrict conv_tty);
static void
sbuild_auth_conv_tty_set_fatal_timeout (SbuildAuthConvTty *conv_tty,
					time_t             timeout);
static gboolean
sbuild_auth_conv_tty_conversation (SbuildAuthConvTty *conv_tty,
				   guint num_messages,
				   SbuildAuthMessageVector *messages);

enum
{
  PROP_0,
  PROP_WARNING_TIMEOUT,
  PROP_FATAL_TIMEOUT
};

static GObjectClass *parent_class;

static void
sbuild_auth_conv_tty_interface_init (gpointer g_iface,
				     gpointer iface_data)
{
  SbuildAuthConvInterface *iface = (SbuildAuthConvInterface *) g_iface;

  iface->get_warning_timeout =
    (SbuildAuthConvGetWarningTimeoutFunc) sbuild_auth_conv_tty_get_warning_timeout;
  iface->set_warning_timeout =
    (SbuildAuthConvSetWarningTimeoutFunc) sbuild_auth_conv_tty_set_warning_timeout;
  iface->get_fatal_timeout =
    (SbuildAuthConvGetFatalTimeoutFunc) sbuild_auth_conv_tty_get_fatal_timeout;
  iface->set_fatal_timeout =
    (SbuildAuthConvSetFatalTimeoutFunc) sbuild_auth_conv_tty_set_fatal_timeout;
  iface->conversation =
    (SbuildAuthConvConversationFunc) sbuild_auth_conv_tty_conversation;
}

G_DEFINE_TYPE_WITH_CODE(SbuildAuthConvTty, sbuild_auth_conv_tty, G_TYPE_OBJECT,
			G_IMPLEMENT_INTERFACE(SBUILD_TYPE_AUTH_CONV,
					      sbuild_auth_conv_tty_interface_init))

/**
 * sbuild_auth_conv_tty_new:
 *
 * Creates a new #SbuildAuthConvTty.  The warning timeout and fatal
 * timeout are initially set to 0 (as per the property defaults).
 *
 * Returns the newly created #SbuildAuthConvTty.
 */
SbuildAuthConvTty *
sbuild_auth_conv_tty_new (void)
{
  return (SbuildAuthConvTty *) g_object_new(SBUILD_TYPE_AUTH_CONV_TTY,
					    NULL);
}

/**
 * sbuild_auth_conv_tty_get_warning_timeout:
 * @conv_tty: an #SbuildAuthConvTty.
 *
 * Get the warning timeout for @conv_tty.
 *
 * Returns the timeout.
 */
static time_t
sbuild_auth_conv_tty_get_warning_timeout (const SbuildAuthConvTty *restrict conv_tty)
{
  g_return_val_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty), 0);

  return conv_tty->warning_timeout;
}

/**
 * sbuild_auth_conv_tty_set_warning_timeout:
 * @conv_tty: an #SbuildAuthConvTty.
 * @timeout: the timeout to set.
 *
 * Set the warning timeout for @conv_tty.
 */
static void
sbuild_auth_conv_tty_set_warning_timeout (SbuildAuthConvTty *conv_tty,
					  time_t             timeout)
{
  g_return_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty));

  conv_tty->warning_timeout = timeout;
  g_object_notify(G_OBJECT(conv_tty), "warning-timeout");
}

/**
 * sbuild_auth_conv_tty_get_fatal_timeout:
 * @conv_tty: an #SbuildAuthConvTty.
 *
 * Get the fatal timeout for @conv_tty.
 *
 * Returns the timeout.
 */
static time_t
sbuild_auth_conv_tty_get_fatal_timeout (const SbuildAuthConvTty *restrict conv_tty)
{
  g_return_val_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty), 0);

  return conv_tty->fatal_timeout;
}

/**
 * sbuild_auth_conv_tty_set_fatal_timeout:
 * @conv_tty: an #SbuildAuthConvTty.
 * @timeout: the timeout to set.
 *
 * Set the fatal timeout for @conv_tty.
 */
static void
sbuild_auth_conv_tty_set_fatal_timeout (SbuildAuthConvTty *conv_tty,
					time_t             timeout)
{
  g_return_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty));

  conv_tty->fatal_timeout = timeout;
  g_object_notify(G_OBJECT(conv_tty), "fatal-timeout");
}

static volatile gboolean timer_expired = FALSE;

/**
 * sbuild_auth_conv_tty_reset_alarm:
 * @conv_tty: an #SbuildAuthConvTty.
 *
 * Cancel any alarm set previously, and restore the state of SIGALRM
 * prior to the conversation.
 */
static void
sbuild_auth_conv_tty_reset_alarm (struct sigaction *orig_sa)
{
  /* Stop alarm */
  alarm (0);
 /* Restore original handler */
  sigaction (SIGALRM, orig_sa, NULL);
}

/**
 * sbuild_auth_conv_tty_alarm_handler:
 * @conv_tty: an #SbuildAuthConvTty.
 *
 * Handle the SIGALRM signal.
 */
static void
sbuild_auth_conv_tty_alarm_handler (gint ignore)
{
  timer_expired = TRUE;
}

/**
 * sbuild_auth_conv_tty_set_alarm:
 * @conv_tty: an #SbuildAuthConvTty.
 * @orig_sa: the original signal handler
 *
 * Set the SIGALARM handler, and set the timeout to @delay seconds.
 * The old signal handler is stored in @orig_sa.
 */
static gboolean
sbuild_auth_conv_tty_set_alarm (gint              delay,
				struct sigaction *orig_sa)
{
  struct sigaction new_sa;
  sigemptyset(&new_sa.sa_mask);
  new_sa.sa_flags = 0;
  new_sa.sa_handler = sbuild_auth_conv_tty_alarm_handler;

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
 * sbuild_auth_conv_tty_get_delay:
 * @conv_tty: an #SbuildAuthConvTty.
 *
 * Get the time delay before the next SIGALRM signal.  If either the
 * warning timeout or the fatal timeout have expired, a message to
 * notify the user is printed to stderr.
 *
 * Returns the delay in seconds, 0 if no delay is set, or -1 if the
 * fatal timeout has expired.
 */
static gint
sbuild_auth_conv_tty_get_delay (SbuildAuthConvTty *conv_tty)
{
  g_return_val_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty), 0);

  timer_expired = 0;
  time (&conv_tty->start_time);

  if (conv_tty->fatal_timeout != 0 &&
      conv_tty->start_time >= conv_tty->fatal_timeout)
    {
      fprintf(stderr, _("Timed out\n"));
      return -1;
    }

  if (conv_tty->warning_timeout != 0 &&
      conv_tty->start_time >= conv_tty->warning_timeout)
    {
      fprintf(stderr, _("Time is running out...\n"));
      return (conv_tty->fatal_timeout ?
	      conv_tty->fatal_timeout - conv_tty->start_time : 0);
    }

  if (conv_tty->warning_timeout != 0)
    return conv_tty->warning_timeout - conv_tty->start_time;
  else if (conv_tty->fatal_timeout != 0)
    return conv_tty->fatal_timeout - conv_tty->start_time;
  else
    return 0;
}

/**
 * sbuild_auth_conv_tty_read_string:
 * @conv_tty: an #SbuildAuthConvTty.
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
static gchar *
sbuild_auth_conv_tty_read_string (SbuildAuthConvTty *conv_tty,
				  const char        *message,
				  gboolean           echo)
{
  struct termios orig_termios, noecho_termios;
  struct sigaction saved_signals;
  sigset_t old_sigs, new_sigs;
  gboolean use_termios = FALSE;
  gchar *return_input = NULL;

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

  gint delay = sbuild_auth_conv_tty_get_delay(conv_tty);

  while (delay >= 0)
    {
      fprintf(stderr, "%s", message);

      if (use_termios == TRUE)
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &noecho_termios);

      if (delay > 0 && sbuild_auth_conv_tty_set_alarm(delay, &saved_signals) == FALSE)
	break;
      else
	{
	  int nchars = read(STDIN_FILENO, input, PAM_MAX_MSG_SIZE - 1);
	  if (use_termios)
	    {
	      tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
	      if (echo == FALSE && timer_expired == TRUE)
		fprintf(stderr, "\n");
	    }
	  if (delay > 0)
	    sbuild_auth_conv_tty_reset_alarm(&saved_signals);
	  if (timer_expired == TRUE)
	    {
	      delay = sbuild_auth_conv_tty_get_delay(conv_tty);
	    }
	  else if (nchars > 0)
	    {
	      if (echo == FALSE)
		fprintf(stderr, "\n");

	      if (input[nchars-1] == '\n')
		input[--nchars] = '\0';
	      else
		input[nchars] = '\0';

	      return_input = g_strdup(input);
	      break;
	    }
	  else if (nchars == 0)
	    {
	      if (echo == FALSE)
		fprintf(stderr, "\n");

	      return_input = g_strdup("");
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
 * sbuild_auth_conv_tty_conversation:
 * @conv_tty: an #SbuildAuthConvTty.
 * @num_messages: the number of messages
 * @messages: the messages to display to the user
 *
 * Hold a conversation with the user.
 *
 * Returns TRUE on success, FALSE on failure.
 */
static gboolean
sbuild_auth_conv_tty_conversation (SbuildAuthConvTty *conv_tty,
				   guint num_messages,
				   SbuildAuthMessageVector *messages)
{
  g_return_val_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty), FALSE);

  for (guint i = 0; i < num_messages; ++i)
    {
      SbuildAuthMessage *msg = messages->messages[i];

      switch (msg->type)
	{
	case SBUILD_AUTH_MESSAGE_PROMPT_NOECHO:
	  msg->response = sbuild_auth_conv_tty_read_string(conv_tty, msg->message, FALSE);
	  if (msg->response == NULL)
	    return FALSE;
	  break;
	case SBUILD_AUTH_MESSAGE_PROMPT_ECHO:
	  msg->response = sbuild_auth_conv_tty_read_string(conv_tty, msg->message, TRUE);
	  if (msg->response == NULL)
	    return FALSE;
	  break;
	case SBUILD_AUTH_MESSAGE_ERROR:
	  if (fprintf(stderr, "%s\n", msg->message) < 0)
	    return FALSE;
	  break;
	case SBUILD_AUTH_MESSAGE_INFO:
	  if (fprintf(stdout, "%s\n", msg->message) < 0)
	    return FALSE;
	  break;
	default:
	  fprintf(stderr, "Unsupported conversation type %d\n", msg->type);
	  return FALSE;
	  break;
	}
    }

  return TRUE;
}


static void
sbuild_auth_conv_tty_init (SbuildAuthConvTty *conv_tty)
{
  g_return_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty));

  conv_tty->warning_timeout = 0;
  conv_tty->fatal_timeout = 0;
  conv_tty->start_time = 0;
}

static void
sbuild_auth_conv_tty_finalize (SbuildAuthConvTty *conv_tty)
{
  g_return_if_fail(SBUILD_IS_AUTH_CONV_TTY(conv_tty));

  if (parent_class->finalize)
    parent_class->finalize(G_OBJECT(conv_tty));
}

static void
sbuild_auth_conv_tty_set_property (GObject      *object,
				   guint         param_id,
				   const GValue *value,
				   GParamSpec   *pspec)
{
  SbuildAuthConvTty *conv_tty;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_AUTH_CONV_TTY (object));

  conv_tty = SBUILD_AUTH_CONV_TTY(object);

  switch (param_id)
    {
    case PROP_WARNING_TIMEOUT:
      sbuild_auth_conv_tty_set_warning_timeout(conv_tty, g_value_get_uint(value));
      break;
    case PROP_FATAL_TIMEOUT:
      sbuild_auth_conv_tty_set_fatal_timeout(conv_tty, g_value_get_uint(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_auth_conv_tty_get_property (GObject    *object,
				   guint       param_id,
				   GValue     *value,
				   GParamSpec *pspec)
{
  SbuildAuthConvTty *conv_tty;

  g_return_if_fail (object != NULL);
  g_return_if_fail (SBUILD_IS_AUTH_CONV_TTY (object));

  conv_tty = SBUILD_AUTH_CONV_TTY(object);

  switch (param_id)
    {
    case PROP_WARNING_TIMEOUT:
      g_value_set_uint(value, conv_tty->warning_timeout);
      break;
    case PROP_FATAL_TIMEOUT:
      g_value_set_uint(value, conv_tty->fatal_timeout);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
sbuild_auth_conv_tty_class_init (SbuildAuthConvTtyClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = (GObjectFinalizeFunc) sbuild_auth_conv_tty_finalize;
  gobject_class->set_property = (GObjectSetPropertyFunc) sbuild_auth_conv_tty_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) sbuild_auth_conv_tty_get_property;
  g_object_class_override_property (gobject_class,
				    PROP_WARNING_TIMEOUT, "warning-timeout");
  g_object_class_override_property (gobject_class,
				    PROP_FATAL_TIMEOUT, "fatal-timeout");
}

/*
 * Local Variables:
 * mode:C
 * End:
 */
