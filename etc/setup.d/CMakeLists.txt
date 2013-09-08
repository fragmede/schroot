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

set(setup_data
    common-config
    common-data
    common-functions)

set(setup_scripts
    00check
    05file
    05btrfs
    05lvm
    05union
    10mount
    15binfmt
    15killprocs
    20copyfiles
    20nssdatabases
    50chrootname
    70services)

install(FILES ${setup_data}
        DESTINATION ${SCHROOT_SETUP_DATA_DIR})

install(PROGRAMS ${setup_scripts}
        DESTINATION ${SCHROOT_CONF_SETUP_D})

install(CODE "
message(STATUS \"Symlinking: \$ENV{DESTDIR}${SCHROOT_CONF_SETUP_D}/99check\")
execute_process(COMMAND ln -sf 00check \$ENV{DESTDIR}${SCHROOT_CONF_SETUP_D}/99check)
")
