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
 * substitutes Boost types as TR1 types when the Standard Library does
 * not support TR1.
 */

#ifndef SBUILD_TR1TYPES_H
#define SBUILD_TR1TYPES_H

#include <sbuild/sbuild-config.h>

#ifdef HAVE_TR1_MEMORY
#include <tr1/memory>
#elif HAVE_BOOST_SHARED_PTR_HPP
#include <boost/shared_ptr.hpp>
namespace std {
  namespace tr1 {
    using boost::shared_ptr;
    using boost::static_pointer_cast;
    using boost::const_pointer_cast;
    using boost::dynamic_pointer_cast;
  }
}
#else
#error A shared_ptr implementation is not available
#endif

#ifdef HAVE_TR1_TUPLE
#include <tr1/tuple>
#elif HAVE_BOOST_TUPLE_TUPLE_HPP
#include <boost/tuple/tuple.hpp>
namespace std { namespace tr1 { using boost::tuple; using boost::get; } }
#else
#error A tuple implementation is not available
#endif

#endif /* SBUILD_TR1TYPES_H */

/*
 * Local Variables:
 * mode:C++
 * End:
 */
