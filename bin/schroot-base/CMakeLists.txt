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

set(schroot_base_sources
    schroot-base-main.h
    schroot-base-main.cc
    schroot-base-option-action.h
    schroot-base-option-action.cc
    schroot-base-options.h
    schroot-base-options.cc
    schroot-base-run.h)

add_library(schroot-base STATIC ${schroot_base_sources})
target_link_libraries(schroot-base sbuild ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE})
