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

/**
 * @file sbuild-tr1types.h TR1 type substitution.  This header
 * substitutes TR1 types or their equivalent Boost types for the same
 * types in the std namespace when not using a conforming C++11
 * compiler.  This permits all code to use the C++11 standard types
 * irrespective of the compiler being used.
 */

#ifndef SBUILD_TR1TYPES_H
# define SBUILD_TR1TYPES_H

# include <sbuild/sbuild-config.h>

# ifdef HAVE_MEMORY_SHARED_PTR
#  include <memory>
# elif HAVE_TR1_MEMORY
# include <tr1/memory>
namespace std {
    using std::tr1::shared_ptr;
    using std::tr1::weak_ptr;
    using std::tr1::static_pointer_cast;
    using std::tr1::const_pointer_cast;
    using std::tr1::dynamic_pointer_cast;
}
# elif HAVE_BOOST_SHARED_PTR_HPP
#  include <boost/shared_ptr.hpp>
namespace std {
    using boost::shared_ptr;
    using boost::weak_ptr;
    using boost::static_pointer_cast;
    using boost::const_pointer_cast;
    using boost::dynamic_pointer_cast;
}
# else
#  error A shared_ptr implementation is not available
# endif

# ifdef HAVE_TUPLE
#  include <tuple>
# elif HAVE_TR1_TUPLE
#  include <tr1/tuple>
namespace std {
  using tr1::tuple;
  using tr1::get;
}
# elif HAVE_BOOST_TUPLE_TUPLE_HPP
#  include <boost/tuple/tuple.hpp>
namespace std {
  using boost::tuple;
  using boost::get;
}
# else
#  error A tuple implementation is not available
# endif

#endif /* SBUILD_TR1TYPES_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
