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

set(listmounts_sources
    schroot-listmounts-main.h
    schroot-listmounts-main.cc
    schroot-listmounts-options.h
    schroot-listmounts-options.cc
    schroot-listmounts.cc)

add_executable(schroot-listmounts ${listmounts_sources})
target_link_libraries(schroot-listmounts sbuild schroot-base)

install(TARGETS schroot-listmounts RUNTIME
        DESTINATION ${SCHROOT_LIBEXEC_DIR})
