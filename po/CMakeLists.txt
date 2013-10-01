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

if(BUILD_NLS)

  set(domain schroot)
  set(xgettext_options --keyword=_ --keyword=N_ --from-code=UTF-8
      --boost --no-location)
  set(copyright_holder "Roger Leigh <rleigh@debian.org>")
  set(bug_address "Roger Leigh <rleigh@debian.org>")

  file(READ LINGUAS languages)
  STRING(REGEX REPLACE "\n" ";" languages "${languages}")
  foreach(lang ${languages})
    set(po_files ${po_files} "${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po")
  endforeach(lang)

  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in potfiles)
  STRING(REGEX REPLACE "\n" ";" potfiles "${potfiles}")
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/POTFILES)
  foreach(file ${potfiles})
    set(translated_files ${translated_files} "${PROJECT_SOURCE_DIR}/${file}")
  endforeach(file ${potfiles})

  foreach(file ${translated_files})
    if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/POTFILES)
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/POTFILES "${file}\n")
    else(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/POTFILES)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/POTFILES "${file}\n")
    endif(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/POTFILES)
  endforeach(file ${translated_files})

  # xgettext creates schroot.pot
  if(GIT_RELEASE_ENABLE)
    # Always update potfile when dependencies change
    add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot"
                       COMMAND xgettext --default-domain=${domain} --add-comments=TRANSLATORS:
                         ${xgettext_options} --files-from "${CMAKE_CURRENT_BINARY_DIR}/POTFILES"
                         --copyright-holder=${copyright_holder}
                         --msgid-bugs-address=${bug_address}
                         -d "${domain}" -p "${CMAKE_CURRENT_SOURCE_DIR}"
                       COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.po" "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot"
                       DEPENDS ${translated_files}
                       VERBATIM)
    add_custom_target(update-pot DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot")
  else(GIT_RELEASE_ENABLE)
    # Only update potfile manually
    add_custom_target(update-pot
                      COMMAND xgettext --default-domain=${domain} --add-comments=TRANSLATORS:
                        ${xgettext_options} --files-from "${CMAKE_CURRENT_BINARY_DIR}/POTFILES"
                        --copyright-holder=${copyright_holder}
                        --msgid-bugs-address=${bug_address}
                        -d "${domain}" -p "${CMAKE_CURRENT_SOURCE_DIR}"
                      COMMAND ${CMAKE_COMMAND} -E rename "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.po" "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot"
                      DEPENDS ${translated_files}
                      VERBATIM)
  endif(GIT_RELEASE_ENABLE)

  # msgmerge --update
  foreach(pofile ${po_files})
    add_custom_command(OUTPUT "${pofile}"
                       COMMAND msgmerge --update "${pofile}"
                         "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot"
                       COMMAND touch "${pofile}"
                       DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${domain}.pot"
                       VERBATIM)
  endforeach(pofile)
  add_custom_target(update-po DEPENDS ${po_files})

  # msgfmt
  foreach(lang ${languages})
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo"
                       COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo"
                       COMMAND msgfmt -c --statistics -o "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo"
                         "${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po"
                       COMMAND touch "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo"
                       DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po"
                       VERBATIM)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo"
            DESTINATION "${SCHROOT_LOCALE_DIR}/${lang}/LC_MESSAGES"
            RENAME "${domain}.mo")
    set(gmo_files ${gmo_files} "${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo")
  endforeach(lang)

if(GIT_RELEASE_ENABLE)
  add_dependencies(git-release update-pot update-po)
endif(GIT_RELEASE_ENABLE)

  add_custom_target(update-gmo ALL DEPENDS ${gmo_files})

  add_custom_target(po-notify
                    COMMAND podebconf-report-po --call --withtranslators --noforce --podir=${CMAKE_CURRENT_SOURCE_DIR})

endif(BUILD_NLS)
