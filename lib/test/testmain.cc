/* Copyright © 2006-2013  Roger Leigh <rleigh@debian.org>
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

#include <sbuild/log.h>

#include <gtest/gtest.h>

int
main(int   argc,
     char *argv[])
{
#ifdef SBUILD_DEBUG
  sbuild::debug_log_level = sbuild::DEBUG_NOTICE;
#else
  sbuild::debug_log_level = sbuild::DEBUG_NONE;
#endif

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
