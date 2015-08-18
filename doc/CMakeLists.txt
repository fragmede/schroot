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

if(BUILD_DOXYGEN)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sbuild.dox.cmake
                 ${CMAKE_CURRENT_BINARY_DIR}/sbuild.dox @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/schroot.dox.cmake
                 ${CMAKE_CURRENT_BINARY_DIR}/schroot.dox @ONLY)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sbuild.log
                     COMMAND ${CMAKE_COMMAND} -E remove_directory sbuild
                     COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/sbuild.dox
                     COMMAND ${CMAKE_COMMAND} -E echo "══════════ UNDOCUMENTED CODE ══════════"
                     COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/sbuild.log
                     COMMAND ${CMAKE_COMMAND} -E echo "═════════ END UNDOCUMENTED CODE ═══════"
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sbuild.dox)

  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/schroot.log
                     COMMAND ${CMAKE_COMMAND} -E remove_directory schroot
                     COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/schroot.dox
                     COMMAND ${CMAKE_COMMAND} -E echo "══════════ UNDOCUMENTED CODE ══════════"
                     COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/schroot.log
                     COMMAND ${CMAKE_COMMAND} -E echo "═════════ END UNDOCUMENTED CODE ═══════"
                     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/schroot.dox)

  add_custom_target(doc DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sbuild.log ${CMAKE_CURRENT_BINARY_DIR}/schroot.log)
endif(BUILD_DOXYGEN)
