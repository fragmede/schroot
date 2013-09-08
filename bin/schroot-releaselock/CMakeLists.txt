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

set(releaselock_sources
    schroot-releaselock-main.h
    schroot-releaselock-main.cc
    schroot-releaselock-options.h
    schroot-releaselock-options.cc
    schroot-releaselock.cc)

add_executable(schroot-releaselock ${releaselock_sources})
target_link_libraries(schroot-releaselock sbuild schroot-base
                      ${Boost_FILESYSTEM_LIBRARY_RELEASE}
                      ${Boost_SYSTEM_LIBRARY_RELEASE})

install(TARGETS schroot-releaselock RUNTIME
        DESTINATION ${SCHROOT_LIBEXEC_DIR})
