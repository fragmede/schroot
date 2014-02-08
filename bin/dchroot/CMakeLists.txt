# Copyright © 2004-2013  Roger Leigh <rleigh@debian.org>
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

set(dchroot_sources
    session.h
    session.cc
    options.h
    options.cc
    main.h
    main.cc
    dchroot.cc)

include_directories(${PROJECT_BINARY_DIR}/bin ${PROJECT_SOURCE_DIR}/bin)

if(BUILD_DCHROOT)
  add_executable(dchroot ${dchroot_sources})
  target_link_libraries(dchroot libschroot bin-common schroot-common dchroot-common)

  install(TARGETS dchroot RUNTIME
          DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                       GROUP_READ GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE
                      SETUID)
endif(BUILD_DCHROOT)
