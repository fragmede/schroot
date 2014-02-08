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

/**
 * @file i18n.h Internationalisation functions.  This header
 * defines the functions used to mark up and translate strings.
 */

#ifndef SBUILD_I18N_H
#define SBUILD_I18N_H

#include <sbuild/config.h>

#include <string>

#ifdef SBUILD_FEATURE_NLS
# include <libintl.h>
#endif // SBUILD_FEATURE_NLS

// Undefine macros which would interfere with our functions.
#ifdef gettext
#undef gettext
#endif
#ifdef _
#undef _
#endif
#ifdef gettext_noop
#undef gettext_noop
#endif
#ifdef N_
#undef N_
#endif

namespace sbuild
{

  /**
   * Bind a text message domain.
   *
   * Note the returned directory may be altered from the directory
   * requested.
   *
   * @param domain the text message domain.
   * @param directory the directory containing the message catalogues.
   * @returns the directory or null on failure.
   *
   * @todo Throw exception on failure.
   */
  inline const char *
  bindtextdomain (const char *domain,
                  const char *directory)
  {
#ifdef SBUILD_FEATURE_NLS
    return ::bindtextdomain(domain, directory);
#else
    return directory;
#endif // SBUILD_FEATURE_NLS
  }

  /**
   * Bind a text message domain.
   *
   * Note the returned directory may be altered from the directory
   * requested.
   *
   * @param domain the text message domain.
   * @param directory the directory containing the message catalogues.
   * @returns the directory or null on failure.
   *
   * @todo Throw exception on failure.
   */
  inline const char *
  bindtextdomain (const std::string& domain,
                  const std::string& directory)
  {
    return bindtextdomain(domain.c_str(), directory.c_str());
  }

  /**
   * Set text message domain.
   *
   * Note the returned domain may differ from the domain requested.
   *
   * @param domain the text message domain.
   * @returns the message domain set.
   *
   * @todo Throw exception on failure.
   */
  inline const char *
  textdomain (const char *domain)
  {
#ifdef SBUILD_FEATURE_NLS
    return ::textdomain(domain);
#else
    return domain;
#endif // SBUILD_FEATURE_NLS
  }

  /**
   * Set text message domain.
   *
   * Note the returned domain may differ from the domain requested.
   *
   * @param domain the text message domain.
   * @returns the message domain set.
   *
   * @todo Throw exception on failure.
   */
  inline const char *
  bindtextdomain (const std::string& domain)
  {
    return textdomain(domain.c_str());
  }

  /**
   * Get a translated message.
   *
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  gettext (const char *message)
  {
#ifdef SBUILD_FEATURE_NLS
    return ::dgettext (SBUILD_MESSAGE_CATALOGUE, message);
#else
    return message;
#endif // SBUILD_FEATURE_NLS
  }

  /**
   * Get a translated message.
   *
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  gettext (const std::string& message)
  {
    return gettext(message.c_str());
  }

  /**
   * Get a translated message from the specified message domain.
   *
   * @param domain the message domain.
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  dgettext (const char *domain,
            const char *message)
  {
#ifdef SBUILD_FEATURE_NLS
    return ::dgettext (domain, message);
#else
    return message;
#endif // SBUILD_FEATURE_NLS
  }

  /**
   * Get a translated message from the specified message domain.
   *
   * @param domain the message domain.
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  dgettext (const char        *domain,
            const std::string& message)
  {
    return dgettext(domain, message.c_str());
  }

  /**
   * Get a translated message from the specified message domain.
   *
   * @param domain the message domain.
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  dgettext (const std::string& domain,
            const std::string& message)
  {
    return dgettext(domain.c_str(), message.c_str());
  }

  /**
   * Get a translated message.  This function is a shorthand for
   * gettext, which also marks up the string for translation.
   *
   * @param message the message to translate.
   * @returns the translated message.
   */
  inline const char *
  _ (const char *message)
  {
    return gettext (message);
  }

  /**
   * Get a message with no translation.
   *
   * @param message the message to not translate.
   * @returns the message.
   */
  inline const char *
  gettext_noop (const char *message)
  {
    return message;
  }

  /**
   * Get a message with no translation.  This macro is a shorthand for
   * gettext_noop, which also marks up the string for translation.
   *
   * @param message the message to not translate.
   * @returns the message.
   */
  inline const char *
  N_ (const char *message)
  {
    return gettext_noop (message);
  }

}

#endif /* SBUILD_I18N_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
