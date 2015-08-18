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

find_program(PO4A_EXECUTABLE po4a)
find_program(GROFFER_EXECUTABLE groffer)
find_program(SOELIM_EXECUTABLE soelim)

# generate config.man
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.man.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config.man)

set(common_dependencies
    authors.man
    copyright.man)

set(manpage_sources
    dchroot.1.man
    dchroot-dsa.1.man
    schroot.1.man
    schroot-setup.5.man
    schroot.conf.5.man
    schroot-script-config.5.man
    schroot-faq.7.man)

# Translated manual pages

file(READ po/LINGUAS languages)
STRING(REGEX REPLACE "\n" ";" languages "${languages}")
foreach(lang ${languages})
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po")
    set(po_sources ${po_sources} "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po")
  endif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po")
endforeach(lang)

set(po4a_command
    ${PO4A_EXECUTABLE} --package-name ${CMAKE_PROJECT_NAME}
    --package-version ${GIT_RELEASE_VERSION}
    --copyright-holder "Roger Leigh <rleigh@codelibre.net>"
    "--srcdir=${CMAKE_CURRENT_SOURCE_DIR}"
    "--destdir=${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/po4a.cfg")

if(PO4A_EXECUTABLE)
  # We need to run po4a initially to see what it generates, prior to
  # generating the rule to autogenerate from source.
  execute_process(COMMAND ${po4a_command})

  file(GLOB translated_dirs RELATIVE
       ${CMAKE_CURRENT_BINARY_DIR}/translated
       ${CMAKE_CURRENT_BINARY_DIR}/translated/*)
  foreach(lang ${translated_dirs})
    if(IS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/translated/${lang}")
      set(languages {languages} ${lang})
      file(GLOB translated_files
           ${CMAKE_CURRENT_BINARY_DIR}/translated/${lang}/*.man)
        set(translated_manpage_sources
            ${translated_manpage_sources}
            ${translated_files})
    endif(IS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/translated/${lang}")
  endforeach(lang)

  add_custom_command(OUTPUT ${translated_manpage_sources}
                     COMMAND ${po4a_command}
                     DEPENDS ${common_dependencies} ${manpage_sources}
                             ${po_sources}
                             "${CMAKE_CURRENT_SOURCE_DIR}/po4a.cfg"
                     VERBATIM)
else(PO4A_EXECUTABLE)
  message(WARNING "po4a not available; not translating manual pages")
endif(PO4A_EXECUTABLE)

set(manpage_sources
    ${manpage_sources}
    ${translated_manpage_sources})

add_custom_target(manpage-sources ALL DEPENDS ${manpage_sources})
if(GIT_RELEASE_ENABLE)
  add_dependencies(git-release manpage-sources)
endif(GIT_RELEASE_ENABLE)

foreach(manpage_source ${manpage_sources})
  unset(lang)
  unset(pagename)
  unset(section)
  string(REGEX MATCH ".*/translated/([^/]+)/(.+)\\.([0-9])\\.man\$" is_translation ${manpage_source})
  if(is_translation)
    string(REGEX REPLACE ".*/translated/([^/]+)/(.+)\\.([0-9])\\.man\$" "\\1;\\2;\\3" translation_matches ${manpage_source})
    list(GET translation_matches 0 lang)
    list(GET translation_matches 1 pagename)
    list(GET translation_matches 2 section)
      set(manpage_source_file ${manpage_source})
  else(is_translation)
    string(REGEX MATCH "^(.+)\\.([0-9])\\.man\$" is_manpage ${manpage_source})
    if (is_manpage)
      string(REGEX REPLACE "^(.+)\\.([0-9])\\.man\$" "\\1;\\2" manpage_matches ${manpage_source})
      unset(lang)
      list(GET manpage_matches 0 pagename)
      list(GET manpage_matches 1 section)
      set(manpage_source_file "${CMAKE_CURRENT_SOURCE_DIR}/${manpage_source}")
    endif (is_manpage)
  endif(is_translation)

  if(pagename AND NOT BUILD_DCHROOT)
    string(REGEX MATCH "^dchroot\$" dchroot_match ${pagename})
    if (dchroot_match)
      unset(pagename)
    endif (dchroot_match)
  endif(pagename AND NOT BUILD_DCHROOT)
  if(pagename AND NOT BUILD_DCHROOT_DSA)
    string(REGEX MATCH "^dchroot-dsa\$" dchroot_dsa_match ${pagename})
    if (dchroot_dsa_match)
      unset(pagename)
    endif (dchroot_dsa_match)
  endif(pagename AND NOT BUILD_DCHROOT_DSA)

  if(pagename)
    # Filter out manpages which should not be built or installed.  We
    # have to do this here due to the translated manpage sources being
    # created whether we want all of them or not.

    set(manpage_includes
        -I${CMAKE_CURRENT_BINARY_DIR}
        -I${CMAKE_CURRENT_SOURCE_DIR})
    if (lang)
      set(manpage "${CMAKE_CURRENT_BINARY_DIR}/translated/${lang}/${pagename}.${section}")
      set(manpage_path "${lang}/man${section}")
      set(manpage_includes
          -I${CMAKE_CURRENT_BINARY_DIR}/translated/${lang}
          ${manpage_includes})
    else (lang)
      set(lang "en")
      set(manpage "${CMAKE_CURRENT_BINARY_DIR}/${pagename}.${section}")
      set(manpage_path "man${section}")
    endif (lang)

    add_custom_command(OUTPUT ${manpage}
                       COMMAND ${SOELIM_EXECUTABLE}
                               ${manpage_includes}
                               < "${manpage_source_file}"
                               > "${manpage}"
                       DEPENDS "${manpage_source_file}"
                               ${common_dependencies}
                               ${CMAKE_CURRENT_BINARY_DIR}/config.man
                       VERBATIM)
    install(FILES "${manpage}"
            DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/${manpage_path}")

    set(manpages ${manpages} ${manpage})
    set(schroot-${lang} ${schroot-${lang}} ${manpage})
    list(FIND combined "schroot-${lang}" combined_exists)
    if (combined_exists EQUAL -1)
      set(combined ${combined} "schroot-${lang}")
    endif(combined_exists EQUAL -1)
  endif(pagename)
endforeach(manpage_source)

add_custom_target(manpages ALL DEPENDS ${manpages})

if(GROFFER_EXECUTABLE)
  foreach(manpage ${manpages})
    set(manpages_ps ${manpages_ps} "${manpage}.ps")
    add_custom_command(OUTPUT "${manpage}.ps"
                       COMMAND ${GROFFER_EXECUTABLE} -K utf8 --ps -man --to-stdout "${manpage}" > "${manpage}.ps"
                       DEPENDS "${manpage}" ${common_dependencies} ${CMAKE_CURRENT_BINARY_DIR}/config.man
                       VERBATIM)

    set(manpages_pdf ${manpages_pdf} "${manpage}.pdf")
    add_custom_command(OUTPUT "${manpage}.pdf"
                       COMMAND ${GROFFER_EXECUTABLE} -K utf8 --pdf -man --to-stdout "${manpage}" > "${manpage}.pdf"
                       DEPENDS "${manpage}" ${common_dependencies} ${CMAKE_CURRENT_BINARY_DIR}/config.man
                       VERBATIM)
  endforeach(manpage)

  add_custom_target(manpages-ps DEPENDS ${manpages_ps})
  add_custom_target(manpages-pdf DEPENDS ${manpages_pdf})

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/combined)
  foreach(combined_lang ${combined})
    list(SORT ${combined_lang})

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.ps"
                       COMMAND ${GROFFER_EXECUTABLE} -K UTF8 -T ps -man --to-stdout ${${combined_lang}} > "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.ps"
                       DEPENDS ${${combined_lang}}
                       VERBATIM)
    set(combined_ps ${combined_ps} "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.ps")

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.pdf"
                       COMMAND ${GROFFER_EXECUTABLE} -K UTF8 -T pdf -man --to-stdout ${${combined_lang}} > "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.pdf"
                       DEPENDS ${${combined_lang}}
                       VERBATIM)
    set(combined_pdf ${combined_pdf} "${CMAKE_CURRENT_BINARY_DIR}/combined/${combined_lang}.pdf")

  endforeach(combined_lang)

  add_custom_target(manpages-combined-ps DEPENDS ${combined_ps})
  add_custom_target(manpages-combined-pdf DEPENDS ${combined_pdf})

  add_custom_target(man-po-notify
                    COMMAND ${po4a_command}
                    COMMAND podebconf-report-po --call --withtranslators --noforce --podir=${CMAKE_CURRENT_SOURCE_DIR}/po)

endif(GROFFER_EXECUTABLE)
