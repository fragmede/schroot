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

set(mount_sources
    schroot-mount-main.h
    schroot-mount-main.cc
    schroot-mount-options.h
    schroot-mount-options.cc
    schroot-mount.cc)

add_executable(schroot-mount ${mount_sources})
target_link_libraries(schroot-mount sbuild schroot-base
                      ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                      ${Boost_SYSTEM_LIBRARY_RELEASE})

install(TARGETS schroot-mount RUNTIME
        DESTINATION ${SCHROOT_LIBEXEC_DIR})
