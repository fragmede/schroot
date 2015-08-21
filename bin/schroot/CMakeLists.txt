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

set(schroot_sources
    main.h
    main.cc
    schroot.cc
    options.h
    options.cc)

include_directories(${PROJECT_BINARY_DIR}/bin ${PROJECT_SOURCE_DIR}/bin)
add_executable(schroot ${schroot_sources})
target_link_libraries(schroot
                      libschroot
                      bin-common
                      ${Intl_LIBRARIES})

install(TARGETS schroot RUNTIME
        DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
                    SETUID)

set(installdirs
    ${SCHROOT_CONF_CHROOT_D}
    ${SCHROOT_MOUNT_DIR}
    ${SCHROOT_SESSION_DIR}
    ${SCHROOT_FILE_UNPACK_DIR}
    ${SCHROOT_OVERLAY_DIR}
    ${SCHROOT_UNDERLAY_DIR})

foreach(dir ${installdirs})
  install(CODE "
message(STATUS \"Installing: \$ENV{DESTDIR}${dir}\")
make_directory(\"\$ENV{DESTDIR}${dir}\")
")
endforeach(dir ${installdirs})
