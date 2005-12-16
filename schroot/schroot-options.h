/* schroot-options - schroot options parser
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

#ifndef SBUILD_SCHROOT_OPTIONS_H
#define SBUILD_SCHROOT_OPTIONS_H

#include <string>
#include <vector>

#include "sbuild-session.h"

class SchrootOptions
{
public:
  typedef std::vector<std::string> string_list;

  SchrootOptions(int argc,
		 char *argv[]);
  virtual ~SchrootOptions();

  string_list              chroots;
  string_list              command;
  std::string              user;
  bool                     preserve;
  bool                     quiet;
  bool                     verbose;
  bool                     list;
  bool                     info;
  bool                     all;
  bool                     all_chroots;
  bool                     all_sessions;
  bool                     load_chroots;
  bool                     load_sessions;
  bool                     version;
  SbuildSessionOperation   session_operation;
  bool                     session_force;
};

#endif /* SBUILD_SCHROOT_OPTIONS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
