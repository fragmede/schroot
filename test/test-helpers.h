/* Copyright © 2006  Roger Leigh <rleigh@debian.org>
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

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>

#include <schroot/sbuild-types.h>

using namespace CppUnit;

template<class T>
void test_list(T&                         itype,
	       sbuild::string_list const& list,
	       sbuild::string_list const& (T::*getter)(void) const,
	       void (T::*setter)(sbuild::string_list const&))
{
  // Set items from list.
  (itype.*setter)(list);

  // Check set items exist, but make no assumptions about ordering.
  sbuild::string_list set_items = (itype.*getter)();

  sbuild::string_list orig_list = list;
  sort(orig_list.begin(), orig_list.end());
  sort(set_items.begin(), set_items.end());

  sbuild::string_list missing;
  set_symmetric_difference(orig_list.begin(), orig_list.end(),
			   set_items.begin(), set_items.end(),
			   std::back_inserter(missing));

  if (!missing.empty())
  for (sbuild::string_list::const_iterator pos = missing.begin();
       pos != missing.end();
       ++pos)
    {
      std::cout << "Missing list item: " << *pos << std::endl;
    }
  // Ensure the test is working.
  CPPUNIT_ASSERT(missing.empty());
  CPPUNIT_ASSERT(set_items.size() == list.size());
}

#endif /* TEST_HELPERS_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
