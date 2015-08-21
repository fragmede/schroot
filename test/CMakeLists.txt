# Copyright © 2004-2013  Roger Leigh <rleigh@codelibre.net>
#
# schroot is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# schroot is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#
#####################################################################

include_directories(${GTEST_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/bin)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/schroot)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/schroot/chroot)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin-common)

set(srcdir ${PROJECT_SOURCE_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup-test-data
               ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)

if (BUILD_TESTS)
  add_test(NAME setup-test-data
           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
           COMMAND ${CMAKE_CURRENT_BINARY_DIR}/setup-test-data)
endif (BUILD_TESTS)

add_subdirectory(schroot)
add_subdirectory(bin-common)
