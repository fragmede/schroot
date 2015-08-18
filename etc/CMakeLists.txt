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

set(schroot_sysconf_data
    schroot.conf)

install(FILES ${schroot_sysconf_data}
        DESTINATION ${SCHROOT_SYSCONF_DIR})

set(files
    copyfiles
    fstab
    nssdatabases)

set(profiles
    buildd
    default
    desktop
    minimal
    sbuild)

set(arches
    ${SBUILD_PLATFORM})

foreach(profile ${profiles})
  foreach(file ${files})
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/profiles/${profile}")
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/profiles/${profile}/${file}")
    foreach(arch ${arches})
      if(EXISTS ${output_file})
        file(REMOVE ${output_file})
      endif(EXISTS ${output_file})
      foreach(source_file
        ${CMAKE_CURRENT_SOURCE_DIR}/profile-templates/all/all/${file}
        ${CMAKE_CURRENT_SOURCE_DIR}/profile-templates/all/${arch}/${file}
        ${CMAKE_CURRENT_SOURCE_DIR}/profile-templates/${profile}/all/${file}
        ${CMAKE_CURRENT_SOURCE_DIR}/profile-templates/${profile}/${arch}/${file})
        if(EXISTS ${source_file})
          file(READ ${source_file} content)
          if(EXISTS ${source_file})
            file(APPEND ${output_file} ${content})
          else(EXISTS ${source_file})
            file(WRITE ${output_file} ${content})
          endif(EXISTS ${source_file})
        endif(EXISTS ${source_file})
      endforeach(source_file)
      if(EXISTS ${output_file})
        message(STATUS "Generated schroot profile ${profile}/${file}")
        install(FILES ${output_file}
                DESTINATION ${SCHROOT_SYSCONF_DIR}/${profile})
      endif(EXISTS ${output_file})
    endforeach(arch)
  endforeach(file)
endforeach(profile)

add_subdirectory(pam)
add_subdirectory(bash_completion)
add_subdirectory(setup.d)
