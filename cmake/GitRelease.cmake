# Copyright © 2009-2013  Roger Leigh <rleigh@debian.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#
#####################################################################

# Versioning, releasing and distribution management with git and cmake
# ====================================================================
#
# This facility is a cmake reimplementation of an earlier
# autoconf/automake version from
# http://anonscm.debian.org/gitweb/?p=buildd-tools/schroot.git;a=blob;f=scripts/git-dist.mk;hb=refs/heads/schroot-1.6
# but is further generalised and is rather more configurable, and
# integrates quite seamlessly with cmake.
#
# Most projects currently handle versioning by hardcoding the version
# number in the source repository in some form, for example in the
# cmake scripts, or in the sources or some other file.  This script
# handles release versioning using the metadata in git tags, and for
# distributed release archives, it embeds a generated file so that git
# isn't required to retrieve the release metadata.
#
# This script also handles release and distribution of a project
# entirely within git, which is described in more detail below.
#
# A number of variables customise the behaviour of the script for a
# particular project; set them before including this file.  While the
# defaults will work for most projects, it will typically require
# adapting to the existing conventions used by a particular project.
#
# The intended use of this facility is to replace a manual step in the
# release process with an automated one, such that releasing may be
# done entirely automatically, and the version numbering may be
# controlled at a higher level, for example via a continuous
# integration system such as Jenkins, or by a release manager, etc.
# The versioning is handled external to the source tree, but is done
# entirely within the git repository.
#
# Terms
# -----
#
# Release
#   Tagging the working git branch with a "release tag".  This tag is
#   signed by default, and the naming scheme allows identification of
#   the current or previous release when running cmake.
#
# Distribution
#   Creation of a distributable form of the tagged release.  This
#   involves committing of the tagged release source tree to a
#   "distribution branch" and creation of a "distribution tag".  This
#   tag is signed by default.  The distributed source tree is the
#   release source tree with the addition of release metadata
#   including the version number, and may also include other generated
#   data, e.g. documentation.  This tagged distribution may be
#   subsequently exported as a tarfile or zipfile with "git archive",
#   or used directly from git by downstream consumers.
#
#
# Version metadata
# ----------------
#
# Set GIT_VERSION_FILE, which is the location of a cmake script
# relative to PROJECT_SOURCE_DIR.  The default is "version.cmake".
# For backward compatibility with the preexisting autoconf/automake
# implementation, GIT_VERSION_FILE_COMPAT may also be set, defaulting
# to "VERSION"; this also needs GIT_VERSION_FILE_USE_COMPAT setting to
# ON (default is OFF).
#
# When developing from git, no version file will be used; version
# information will be obtained directly from git.  When using a
# distributed release, version information will be obtained from the
# version file.
#
#
# Release configuration
# ---------------------
#
# This script will not allow tagging of a release if the working git
# branch contains uncommitted changes.  This is in order to prevent
# creation of broken releases—the working tree may appear to build
# and work correctly, but the uncommitted changes won't be tagged.
# Additionally, it will not allow tagging of a release if untracked
# files are present in the tree (for the same reasons--the untracked
# files may require adding and committing).  Set
# GIT_RELEASE_CHECK_UNTRACKED to OFF to allow releasing if untracked
# files are present.
#
# Releasing isn't enabled by default in order to prevent unintentional
# or accidental modification of the git repository.  That is to say,
# making a release must be explicitly requested as a safety check; set
# GIT_RELEASE_ENABLE to ON in order to make a release.
#
# Release tagging is customised using the following options:
#
# GIT_RELEASE_TAG_OPTIONS
#   A list of options to pass to "git tag"; defaults to "-s" to sign
#   the tag.
#
# GIT_RELEASE_TAG_NAME
#   The name of the release tag.  Defaults to
#   "release/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}", but may be
#   changed to just "v${GIT_RELEASE_VERSION}" or any name desired.
#   The only restriction is that it must contain the release version
#   somewhere in the string, and must be a valid git tag name.
#
# GIT_RELEASE_TAG_MATCH
#   A pattern to patch all release versions when using "git describe
#   --match".  Defaults to "release/${CMAKE_PROJECT_NAME}-*",
#   i.e. containing the constant part of GIT_RELEASE_TAG_NAME without
#   the version number.  Must be changed appropriately if
#   GIT_RELEASE_TAG_NAME is customised.
#
# GIT_RELEASE_TAG_REGEX
#   A regular expression to match the release version in the tag name.
#   The release version number must be the first match ("\1").
#   Defaults to "^release/${CMAKE_PROJECT_NAME}-([0-9].*)\$".  Must be
#   changed appropriately if GIT_RELEASE_TAG_NAME is customised.
#
# GIT_RELEASE_TAG_MESSAGE
#   The release tag message.  Any text is permitted here, though it is
#   advisable (but not required) to include GIT_RELEASE_VERSION as
#   part of the message.
#
#
# Releasing
# ---------
#
# Run "cmake" as normal, but add the options:
#
#   -DGIT_RELEASE_ENABLE=ON -DGIT_RELEASE_VERSION=${new_version}
#
# This will provide a new "git-release" target.  If using make, run:
#
#   make git-release
#
# If tag signing is enabled, you'll be prompted for your key
# passphrase.
#
# Run "git log --decorate" or "git describe" and you'll see the
# release tag.  This tag marks the current commit as the release of
# version ${new_version}.
#
#
# Distribution configuration
# --------------------------
#
# Making a distribution of a release requires releasing first (as
# documented above)
#
# Distributing isn't enabled by default in order to prevent
# unintentional or accidental modification of the git repository.
# That is to say, making a distribution must be explicitly requested
# as a safety check; set GIT_DIST_ENABLE to ON in order to make a
# distribution.
#
# Distribution tagging is customised using the following options:
#
# GIT_DIST_TAG_OPTIONS
#   A list of options to pass to "git tag"; defaults to "-s" to sign
#   the tag.
#
# GIT_DIST_TAG_NAME
#   The name of the distribution tag.  Defaults to
#   "distribution/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}", but
#   may be changed to just "v${GIT_RELEASE_VERSION}" or any name
#   desired.  The only restriction is that it must contain the
#   distribution version somewhere in the string, and must be a valid
#   git tag name.
#
# GIT_DIST_TAG_MESSAGE
#   The distribution tag message.  Any text is permitted here, though
#   it is advisable (but not required) to include GIT_DIST_VERSION as
#   part of the message.
#
# Distribution committing and branching is customised using the
# following options:
#
# GIT_DIST_BRANCH
#   The name of the branch to commit onto.  The branch will be created
#   if it does not already exist.  The default is
#   "distribution/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION_MAJOR}.${GIT_RELEASE_VERSION_MINOR}"
#   which is to say that it includes the major and minor version
#   numbers, so that you get a new distribution branch each time the
#   major or minor number is changed, with the patch release versions
#   being committed in sequence on each branch.  This enables
#   downstream tracking of a given release series, and maintenance of
#   multiple release series in parallel.  It is advised to customise
#   this to meet the project's versioning and release strategy such
#   that major stable release series are kept on different branches.
#
# GIT_DIST_COMMIT_MESSAGE
#   The distribution commit message.  Any text is permitted here,
#   though it is advisable (but not required) to include
#   GIT_DIST_VERSION as part of the message.
#
# GIT_DIST_NAME
#   Naming scheme for the release.  This is the name of the release
#   directory, and will be used to prefix the path for release
#   archives.  Defaults to
#   "${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}".
#
# GIT_DIST_TMPDIR
#   Temporary directory used for staging the release distribution tree
#   prior to committing into git.  Defaults to
#   "${PROJECT_BINARY_DIR}/GitRelease" but may be
#   adjusted if this clashes with existing usage of this path.
#
# GIT_DIST_ROOT
#   Root of the staged release distribution.  Defaults to
#   "${GIT_DIST_TMPDIR}/${GIT_DIST_NAME}".
#
#
#
# Distributing
# ------------
#
# Run "cmake" as normal, but add the option:
#
#   -DGIT_DIST_ENABLE=ON
#
# This will provide a new "git-dist" target.  If using make, run:
#
#   make git-dist
#
# If tag signing is enabled, you'll be prompted for your key
# passphrase.
#
# Look at the distribution branch with "git log --decorate --graph" or
# "gitk".  You'll see that the distribution has the tagged release as
# its parent and (if present) the prior distribution.  That is to say,
# the relationships between the releases and distributions are
# explicitly defined.  While this might not be of immediately obvious
# benefit, it means that downstream consumers can make changes and
# push them back to you; merging the changes back upstream is much
# easier since the origin of the changes is fully described by the
# dependency graph.

cmake_policy(SET CMP0007 NEW)

# Settings file used to pass configuration settings
set(GIT_RELEASE_SETTINGS "${PROJECT_BINARY_DIR}/GitRelease.cmake")

# When running in script mode, read the saved settings first
if (CMAKE_SCRIPT_MODE_FILE)
  include("${GIT_RELEASE_SETTINGS}")
endif (CMAKE_SCRIPT_MODE_FILE)

if (NOT CMAKE_SCRIPT_MODE_FILE)
# Version file settings
set(GIT_VERSION_FILE "GitVersion.cmake"
    CACHE FILEPATH "Relative path to cmake version file in distributed source tree")
set(GIT_VERSION_FILE_COMPAT "VERSION"
    CACHE FILEPATH "Relative path to compatibility version file in distributed source tree")
set(GIT_VERSION_FILE_USE_COMPAT OFF
    CACHE BOOL "Create compatibility version file in distributed source tree")
mark_as_advanced(FORCE GIT_VERSION_FILE GIT_VERSION_FILE_COMPAT GIT_VERSION_FILE_USE_COMPAT)

# Release policy settings
set(GIT_RELEASE_POLICY_FILE "GitReleasePolicy.cmake"
    CACHE FILEPATH "Location of release policy configuration")

# Release sanity checking
set(GIT_RELEASE_CHECK_UNCOMMITTED ON
    CACHE BOOL "Check for uncommitted files in working tree")
set(GIT_RELEASE_CHECK_UNTRACKED ON
    CACHE BOOL "Check for untracked files in working tree")
set(GIT_RELEASE_ENABLE OFF
    CACHE BOOL "Enable to permit git-release; not enabled by default for safety")
mark_as_advanced(FORCE GIT_RELEASE_CHECK_UNCOMMITTED GIT_RELEASE_CHECK_UNTRACKED GIT_RELEASE_ENABLE)

# Release matching
set(GIT_RELEASE_TAG_MATCH "release/${CMAKE_PROJECT_NAME}-*"
    CACHE STRING "Pattern match for all release tags for use with \"git describe --match\"")
set(GIT_RELEASE_TAG_REGEX "^release/${CMAKE_PROJECT_NAME}-([0-9].*)\$"
CACHE STRING "Regular expression to extract the version number from the tag name")
mark_as_advanced(FORCE GIT_RELEASE_TAG_MATCH GIT_RELEASE_TAG_REGEX)

# Distribution sanity checking
set(GIT_DIST_ENABLE OFF
    CACHE BOOL "Enable to permit git-dist; not enabled by default for safety")
mark_as_advanced(FORCE GIT_DIST_ENABLE)
endif (NOT CMAKE_SCRIPT_MODE_FILE)

# Split a version number into separate components
# version the version number to split
# major variable name to store the major version in
# minor variable name to store the minor version in
# patch variable name to store the patch version in
# extra variable name to store a version suffix in
function(git_release_version_split version major minor patch extra)
  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)?" version_valid ${version})
  if(version_valid)
    string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)?" "\\1;\\2;\\3;\\4" VERSION_MATCHES ${version})
    list(GET VERSION_MATCHES 0 version_major)
    set(${major} ${version_major} PARENT_SCOPE)
    list(GET VERSION_MATCHES 1 version_minor)
    set(${minor} ${version_minor} PARENT_SCOPE)
    list(GET VERSION_MATCHES 2 version_patch)
    set(${patch} ${version_patch} PARENT_SCOPE)
    list(GET VERSION_MATCHES 3 version_extra)
    set(${extra} ${version_extra} PARENT_SCOPE)
  else(version_valid)
    message(AUTHOR_WARNING "Bad version ${version}; falling back to 0 (have you made an initial release?)")
    set(${major} "0" PARENT_SCOPE)
    set(${minor} "" PARENT_SCOPE)
    set(${patch} "" PARENT_SCOPE)
    set(${extra} "" PARENT_SCOPE)
  endif(version_valid)
endfunction(git_release_version_split)

function(git_release_version_from_file_compat git_version_file)
  if(NOT EXISTS ${git_version_file})
    message(FATAL_ERROR "git version file ${git_version_file} does not exist")
  endif(NOT EXISTS ${git_version_file})
  FILE(READ "${git_version_file}" VERSION_CONTENT)
  STRING(REGEX REPLACE ";" "\\\\;" VERSION_CONTENT "${VERSION_CONTENT}")
  STRING(REGEX REPLACE "\n" ";" VERSION_CONTENT "${VERSION_CONTENT}")

  foreach(line ${VERSION_CONTENT})
    string(REGEX MATCH "^([A-Z][a-zA-Z-]*): (.*)" line_valid ${line})
    if(line_valid)
      string(REGEX REPLACE "^([A-Z][a-zA-Z-]*): (.*)" "\\1;\\2" line_matches ${line})
      list(GET line_matches 0 key)
      list(GET line_matches 1 value)

      if (${key} STREQUAL "Package")
        if (NOT ${CMAKE_PROJECT_NAME} STREQUAL ${value})
          message(FATAL_ERROR "Project name ${CMAKE_PROJECT_NAME} does not match name in ${git_version_file} (${value})")
        endif (NOT ${CMAKE_PROJECT_NAME} STREQUAL ${value})
        set(GIT_RELEASE_PACKAGE ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Package")
      if (${key} STREQUAL "Version")
        set(GIT_RELEASE_VERSION ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Version")
      if (${key} STREQUAL "Release-Date")
        set(GIT_RELEASE_DATE ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Release-Date")
      if (${key} STREQUAL "Release-Date-Unix")
        set(GIT_RELEASE_DATE_UNIX ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Release-Date-Unix")
      if (${key} STREQUAL "Released-By")
        set(GIT_RELEASE_BY ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Released-By")
      if (${key} STREQUAL "Git-Tag")
        set(GIT_RELEASE_TAG ${value} PARENT_SCOPE)
      endif(${key} STREQUAL "Git-Tag")
    endif(line_valid)
  endforeach(line)
endfunction(git_release_version_from_file_compat)

function(git_release_version_from_file git_version_file)
  include("${git_version_file}")

  set(GIT_RELEASE_PACKAGE ${GIT_RELEASE_PACKAGE} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION ${GIT_RELEASE_VERSION} PARENT_SCOPE)
  set(GIT_RELEASE_DATE ${GIT_RELEASE_DATE} PARENT_SCOPE)
  set(GIT_RELEASE_DATE_UNIX ${GIT_RELEASE_DATE_UNIX} PARENT_SCOPE)
  set(GIT_RELEASE_BY ${GIT_RELEASE_BY} PARENT_SCOPE)
  set(GIT_RELEASE_TAG ${GIT_RELEASE_TAG} PARENT_SCOPE)
endfunction(git_release_version_from_file)

function(git_release_version_from_git_tag git_tag tag_exists)
  execute_process(COMMAND git show ${git_tag}
    OUTPUT_VARIABLE tag_content RESULT_VARIABLE show_fail ERROR_QUIET
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (tag_exists AND show_fail)
    message(FATAL_ERROR "Could not obtain release tag ${git_tag} from git")
  endif (tag_exists AND show_fail)
  STRING(REGEX REPLACE ";" "\\\\;" tag_content "${tag_content}")
  STRING(REGEX REPLACE "\n" ";" tag_content "${tag_content}")

  foreach(line ${tag_content})
    string(REGEX MATCH "^Tagger: (.*)" tagger_valid ${line})
    if (tagger_valid)
      string(REGEX REPLACE "^Tagger: (.*)" "\\1" tagger ${line})
    endif (tagger_valid)
  endforeach(line)

  # If tagger is undefined, set to "Unreleased".
  if(NOT tagger)
    set(tagger "Unreleased")
  endif(NOT tagger)
  set(GIT_RELEASE_BY ${tagger} PARENT_SCOPE)

  set(GIT_RELEASE_TAG ${git_tag} PARENT_SCOPE)
  execute_process(COMMAND git rev-parse "${git_tag}^{}"
    OUTPUT_VARIABLE commit RESULT_VARIABLE revparse_fail ERROR_QUIET
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (revparse_fail)
    message(FATAL_ERROR "Could not obtain release commit information from git")
  endif (revparse_fail)
  string(REPLACE "\n" "" commit "${commit}")
  execute_process(COMMAND git log -1 "${commit}" --pretty=%ai
    OUTPUT_VARIABLE commit_date RESULT_VARIABLE revparse_fail ERROR_QUIET
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (revparse_fail)
    message(FATAL_ERROR "Could not obtain release commit date from git")
  endif (revparse_fail)
  string(REPLACE "\n" "" commit_date "${commit_date}")
  execute_process(COMMAND git log -1 "${commit}" --pretty=%at
    OUTPUT_VARIABLE commit_date_unix RESULT_VARIABLE revparse_fail ERROR_QUIET
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (revparse_fail)
    message(FATAL_ERROR "Could not obtain release commit timestamp from git")
  endif (revparse_fail)
  string(REPLACE "\n" "" commit_date_unix "${commit_date_unix}")

  set(GIT_RELEASE_DATE ${commit_date} PARENT_SCOPE)
  set(GIT_RELEASE_DATE_UNIX ${commit_date_unix} PARENT_SCOPE)
  set(GIT_RELEASE_PACKAGE ${CMAKE_PROJECT_NAME} PARENT_SCOPE)

  string(REGEX REPLACE "${GIT_RELEASE_TAG_REGEX}" "\\1" git_release_version ${git_tag})
  set(GIT_RELEASE_VERSION ${git_release_version} PARENT_SCOPE)
endfunction(git_release_version_from_git_tag)

function(git_release_version_from_git tag_match)
  set(tag_exists TRUE)

  execute_process(COMMAND git describe --match "${tag_match}" --exact-match
    OUTPUT_VARIABLE git_tag RESULT_VARIABLE git_fail ERROR_QUIET
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  string(REPLACE "\n" "" git_tag "${git_tag}")
  if(git_fail)
    set(tag_exists FALSE)
    # This commit is not a tagged release;
    execute_process(COMMAND git describe --match "${tag_match}"
      OUTPUT_VARIABLE git_tag RESULT_VARIABLE git_fail2 ERROR_QUIET
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    string(REPLACE "\n" "" git_tag "${git_tag}")
    if(git_fail2)
      message(FATAL_ERROR "Could not obtain release information from git")
    endif(git_fail2)
  endif(git_fail)

  git_release_version_from_git_tag(${git_tag} ${tag_exists})

  set(GIT_RELEASE_PACKAGE ${GIT_RELEASE_PACKAGE} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION ${GIT_RELEASE_VERSION} PARENT_SCOPE)
  set(GIT_RELEASE_DATE ${GIT_RELEASE_DATE} PARENT_SCOPE)
  set(GIT_RELEASE_DATE_UNIX ${GIT_RELEASE_DATE_UNIX} PARENT_SCOPE)
  set(GIT_RELEASE_BY ${GIT_RELEASE_BY} PARENT_SCOPE)
  set(GIT_RELEASE_TAG ${GIT_RELEASE_TAG} PARENT_SCOPE)
endfunction(git_release_version_from_git)

function(git_release_version_to_file_compat git_version_file)
  set(keys
    "Package: ${CMAKE_PROJECT_NAME}"
    "Version: ${GIT_RELEASE_VERSION}")
  if (GIT_RELEASE_DATE)
    set(keys ${keys} "Release-Date: ${GIT_RELEASE_DATE}")
  endif (GIT_RELEASE_DATE)
  if (GIT_RELEASE_DATE_UNIX)
    set(keys ${keys} "Release-Date-Unix: ${GIT_RELEASE_DATE_UNIX}")
  endif (GIT_RELEASE_DATE_UNIX)
  if (GIT_RELEASE_BY)
    set(keys ${keys} "Released-By: ${GIT_RELEASE_BY}")
  endif (GIT_RELEASE_BY)
  if (GIT_RELEASE_TAG)
    set(keys ${keys} "Git-Tag: ${GIT_RELEASE_TAG}")
  endif (GIT_RELEASE_TAG)
  STRING(REGEX REPLACE ";" "\\n" keys "${keys}")

  get_filename_component(dirname ${git_version_file} PATH)
  file(MAKE_DIRECTORY "${dirname}")
  file(WRITE "${git_version_file}" "${keys}\n")
endfunction(git_release_version_to_file_compat)

function(git_release_version_to_file git_version_file)
  set(lines
    "set(GIT_RELEASE_PACKAGE \"${CMAKE_PROJECT_NAME}\")")
  set(lines ${lines}
    "set(GIT_RELEASE_VERSION \"${GIT_RELEASE_VERSION}\")")
  if (GIT_RELEASE_DATE)
   set(lines ${lines}
     "set(GIT_RELEASE_DATE \"${GIT_RELEASE_DATE}\")")
  endif (GIT_RELEASE_DATE)
  if (GIT_RELEASE_DATE_UNIX)
   set(lines ${lines}
     "set(GIT_RELEASE_DATE_UNIX \"${GIT_RELEASE_DATE_UNIX}\")")
  endif (GIT_RELEASE_DATE_UNIX)
  if (GIT_RELEASE_BY)
   set(lines ${lines}
     "set(GIT_RELEASE_BY \"${GIT_RELEASE_BY}\")")
  endif (GIT_RELEASE_BY)
  if (GIT_RELEASE_TAG)
   set(lines ${lines}
     "set(GIT_RELEASE_TAG \"${GIT_RELEASE_TAG}\")")
  endif (GIT_RELEASE_TAG)
  STRING(REGEX REPLACE ";" "\\n" lines "${lines}")

  get_filename_component(dirname ${git_version_file} PATH)
  file(MAKE_DIRECTORY "${dirname}")
  file(WRITE "${git_version_file}" "${lines}\n")
endfunction(git_release_version_to_file)

function(git_release_version tag_match)
  if(GIT_RELEASE_ENABLE OR GIT_DIST_ENABLE)
    if (NOT GIT_RELEASE_VERSION)
      message(FATAL_ERROR "GIT_RELEASE_VERSION not set; required if GIT_RELEASE_ENABLE or GIT_DIST_ENABLE is enabled")
    endif (NOT GIT_RELEASE_VERSION)
  else(GIT_RELEASE_ENABLE OR GIT_DIST_ENABLE)
    if(EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE}")
      message(STATUS "Reading release metadata from ${GIT_VERSION_FILE}")
      git_release_version_from_file("${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE}")
    else(EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE}")
      if(GIT_VERSION_FILE_USE_COMPAT AND EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE_COMPAT}")
      message(STATUS "Reading release metadata from ${GIT_VERSION_FILE_COMPAT}")
        git_release_version_from_file_compat("${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE_COMPAT}")
      else(GIT_VERSION_FILE_USE_COMPAT AND EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE_COMPAT}")
        message(STATUS "Reading release metadata from git")
        git_release_version_from_git("${tag_match}")
      endif(GIT_VERSION_FILE_USE_COMPAT AND EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE_COMPAT}")
    endif(EXISTS "${PROJECT_SOURCE_DIR}/${GIT_VERSION_FILE}")
  endif(GIT_RELEASE_ENABLE OR GIT_DIST_ENABLE)

  git_release_version_split(${GIT_RELEASE_VERSION} major minor patch extra)

  if(NOT GIT_RELEASE_DATE)
    set(GIT_RELEASE_DATE "unreleased")
  endif(NOT GIT_RELEASE_DATE)
  if(NOT GIT_RELEASE_DATE_UNIX)
    set(GIT_RELEASE_DATE_UNIX 0)
  endif(NOT GIT_RELEASE_DATE_UNIX)

  set(GIT_RELEASE_PACKAGE ${GIT_RELEASE_PACKAGE} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION ${GIT_RELEASE_VERSION} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION_MAJOR ${major} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION_MINOR ${minor} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION_PATCH ${patch} PARENT_SCOPE)
  set(GIT_RELEASE_VERSION_EXTRA ${extra} PARENT_SCOPE)
  set(GIT_RELEASE_DATE ${GIT_RELEASE_DATE} PARENT_SCOPE)
  set(GIT_RELEASE_DATE_UNIX ${GIT_RELEASE_DATE_UNIX} PARENT_SCOPE)
  set(GIT_RELEASE_BY ${GIT_RELEASE_BY} PARENT_SCOPE)
  set(GIT_RELEASE_TAG ${GIT_RELEASE_TAG} PARENT_SCOPE)
endfunction(git_release_version)

if (NOT CMAKE_SCRIPT_MODE_FILE)
# Get initial version information
  git_release_version("${GIT_RELEASE_TAG_MATCH}")

set(GIT_RELEASE_VERSION "${GIT_RELEASE_VERSION}"
    CACHE STRING "Release version number")
mark_as_advanced(FORCE GIT_RELEASE_VERSION)

# Release tagging
set(GIT_RELEASE_TAG_OPTIONS "-s"
    CACHE STRING "GPG release tagging options; signing enabled by default")
set(GIT_RELEASE_TAG_NAME "release/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}"
    CACHE STRING "Naming scheme for release tags; must include version number")
set(GIT_RELEASE_TAG_MESSAGE "Release of ${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}"
    CACHE STRING "Message for release tags")
mark_as_advanced(FORCE GIT_RELEASE_TAG_OPTIONS GIT_RELEASE_TAG_NAME GIT_RELEASE_TAG_MESSAGE)

# Distribution tagging
set(GIT_DIST_TAG_OPTIONS "-s"
    CACHE STRING "GPG distribution tagging options; signing enabled by default")
set(GIT_DIST_TAG_NAME distribution/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}
    CACHE STRING "Naming scheme for distribution tags; must include version number")
set(GIT_DIST_TAG_MESSAGE "Distribution of ${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}"
    CACHE STRING "Message for distribution tags")
mark_as_advanced(FORCE GIT_DIST_TAG_OPTIONS GIT_DIST_TAG_NAME GIT_DIST_TAG_MESSAGE)

# Distribution branch commit
set(GIT_DIST_BRANCH distribution/${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION_MAJOR}.${GIT_RELEASE_VERSION_MINOR}
    CACHE STRING "Branch to place distributed release on")
# Message for distribution commit
set(GIT_DIST_COMMIT_MESSAGE "Distribution of ${CMAKE_PROJECT_NAME} version ${GIT_RELEASE_VERSION}"
    CACHE STRING "Message for distribution commit")
# Release directory to distribute
set(GIT_DIST_NAME ${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}
    CACHE STRING "Name of the release distribution; must include version number")
set(GIT_DIST_TMPDIR ${PROJECT_BINARY_DIR}/GitRelease
    CACHE STRING "Temporary staging directory for release")
set(GIT_DIST_ROOT ${GIT_DIST_TMPDIR}/${GIT_DIST_NAME}
    CACHE STRING "Release directory to distribute; must include version number")
mark_as_advanced(FORCE GIT_DIST_BRANCH GIT_DIST_COMMIT_MESSAGE GIT_DIST_NAME GIT_DIST_TMPDIR GIT_DIST_ROOT)
endif (NOT CMAKE_SCRIPT_MODE_FILE)

if(EXISTS "${GIT_RELEASE_POLICY_FILE}")
  include("${GIT_RELEASE_POLICY_FILE}")
endif(EXISTS "${GIT_RELEASE_POLICY_FILE}")

function(git_check_repo)
  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git)
    message(FATAL_ERROR "Source directory ${PROJECT_SOURCE_DIR} is not a git repository")
  endif(NOT EXISTS ${PROJECT_SOURCE_DIR}/.git)
endfunction(git_check_repo)

# Check that the working tree and index are clean prior to making any
# changes.  If dirty, then the changes may be unreproducible and not
# match what was expected.  For example, the distributed files may not
# match those actually committed or may not even be under version
# control.
#
# Project customisation:
# Checking of untracked files may be disabled by setting
# GIT_RELEASE_CHECK_UNTRACKED to OFF.
function(git_check_clean)
  git_check_repo()

  execute_process(COMMAND git diff-index --quiet HEAD
                  RESULT_VARIABLE diff_index_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if(diff_index_status GREATER 1)
    message(FATAL_ERROR "Error checking git index git diff-index: error status ${diff_index_status}")
  endif(diff_index_status GREATER 1)
  if(diff_index_status EQUAL 1)
    if (GIT_RELEASE_CHECK_UNCOMMITTED)
      message(FATAL_ERROR "Uncommitted changes in source directory")
    else (GIT_RELEASE_CHECK_UNCOMMITTED)
      message(WARNING "Uncommitted changes in source directory")
    endif (GIT_RELEASE_CHECK_UNCOMMITTED)
  endif(diff_index_status EQUAL 1)

  if(diff_index_status EQUAL 0 OR diff_index_status EQUAL 1)
    execute_process(COMMAND git ls-files --others --exclude-standard --error-unmatch .
                    OUTPUT_FILE /dev/null
                    ERROR_FILE /dev/null
                    RESULT_VARIABLE ls_files_status
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    # Exit status 1 is OK.
    if(ls_files_status GREATER 1)
      message(FATAL_ERROR "Error checking working directory with git ls-files: error status ${ls_files_status}")
    endif(ls_files_status GREATER 1)
    if(ls_files_status EQUAL 0)
      if (GIT_RELEASE_CHECK_UNTRACKED)
        message(FATAL_ERROR "Untracked files present in working directory")
      else (GIT_RELEASE_CHECK_UNTRACKED)
        message(WARNING "Untracked files present in working directory")
      endif (GIT_RELEASE_CHECK_UNTRACKED)
    endif(ls_files_status EQUAL 0)
  endif(diff_index_status EQUAL 0 OR diff_index_status EQUAL 1)
endfunction(git_check_clean)

# Make a release.
#
# The current working tree is tagged as a new release.  If the release
# tag already exists then the operation will do nothing if the tag
# matches the current working tree, or else it will abort with an
# error.  If the repository has been accidentally tagged previously,
# then remove the tag with "git tag -d TAG" before releasing.
#
# NOTE: Set GIT_RELEASE_ENABLE=true when running make.  This is a
# safety check to avoid accidental damage to the git repository.
#
# NOTE: Running release-git independently of dist-git is NOT
# RECOMMENDED.  The distdir rule can update files in the working tree
# (for example, gettext translations in po/), so running "make
# distdir" prior to tagging the release will ensure the tagged release
# will not differ from the distributed release.
#
# Project customisation:
# The tag will be signed by default; set GIT_RELEASE_TAG_OPTIONS to
# alter.  The tag will be named using GIT_RELEASE_TAG_NAME with the
# GIT_RELEASE_TAG_MESSAGE specifying an appropriate message for the
# tag.
function(git_release fail_if_tag_exists)
  # Check if release tag exists
  execute_process(COMMAND git show-ref --tags -q ${GIT_RELEASE_TAG_NAME}
                  RESULT_VARIABLE show_ref_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (show_ref_status EQUAL 0)
    # Release tag already exists
    if(fail_if_tag_exists)
      message(FATAL_ERROR "git release tag ${GIT_RELEASE_TAG_NAME} already exists; not releasing")
    else(fail_if_tag_exists)
      message(STATUS "git release tag ${GIT_RELEASE_TAG_NAME} found")
    endif(fail_if_tag_exists)
  else(show_ref_status EQUAL 0)
    # Release tag does not exist
    if (NOT GIT_RELEASE_ENABLE)
      message(FATAL_ERROR "GIT_RELEASE_ENABLE not set; not releasing")
    endif (NOT GIT_RELEASE_ENABLE)

    # Check repository is clean to tag
    git_check_clean()

    # Create release tag
    message("Releasing ${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION}")
    message("Creating git release tag ${GIT_RELEASE_TAG_NAME}")
    execute_process(COMMAND git tag -m "${GIT_RELEASE_TAG_MESSAGE}"
                            ${GIT_RELEASE_TAG_OPTIONS} "${GIT_RELEASE_TAG_NAME}"
                            HEAD
                    RESULT_VARIABLE tag_status
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    if(tag_status GREATER 0)
      message(FATAL_ERROR "Failed to create release tag ${GIT_RELEASE_TAG_NAME}: error status ${tag_status}")
    endif(tag_status GREATER 0)

    message(STATUS "${CMAKE_PROJECT_NAME} ${GIT_RELEASE_VERSION} release tagged as ${GIT_RELEASE_TAG_NAME}")
  endif (show_ref_status EQUAL 0)

endfunction(git_release)

# Make a distribution of a release.
#
# A distribution is created and committed onto the specified branch.
# The commit is then tagged.  The distribution commit will have the
# release commit and the previous distribution (if any) as its
# parents.  Thus distribution releases appear to git as merges (with
# the exception of the initial release).
#
# NOTE: Set GIT_DIST_ENABLE=true when running make, plus
# GIT_RELEASE_ENABLE=true if the working tree has not already been
# tagged with a release tag.  This is a safety check to avoid
# accidental damage to the git repository.
#
# Project customisation:
# GIT_DIST_COMMIT_MESSAGE specifies the commit message for the commit,
# and GIT_DIST_BRANCH specifies the branch to add the commit to.
# The tag will be signed by default; set GIT_DIST_TAG_OPTIONS to
# alter.  The tag will be named using GIT_DIST_TAG_NAME with the
# GIT_DIST_TAG_MESSAGE specifying an appropriate message for the
# tag.
function(git_rev_parse ref var)
  execute_process(COMMAND git rev-parse ${ref}
                  OUTPUT_VARIABLE rev_parse_output
                  RESULT_VARIABLE rev_parse_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if(rev_parse_status GREATER 0)
    message(FATAL_ERROR "git rev-parse failed to parse ref ${ref}")
  endif(rev_parse_status GREATER 0)
  string(REPLACE "\n" "" rev_parse_output "${rev_parse_output}")
  set(${var} ${rev_parse_output} PARENT_SCOPE)
endfunction(git_rev_parse)

function(git_archive_tree)
  git_release(OFF)

  git_rev_parse("${GIT_RELEASE_TAG_NAME}^{}" release_commit)
  git_rev_parse(HEAD head_commit)

  if(NOT ${release_commit} STREQUAL ${head_commit})
    message(FATAL_ERROR "Working directory is not at release tag ${GIT_RELEASE_TAG_NAME}^{} (commit ${release_commit}); not distributing")
  endif(NOT ${release_commit} STREQUAL ${head_commit})

  git_check_clean()

  file(REMOVE_RECURSE "${GIT_DIST_TMPDIR}")
  file(MAKE_DIRECTORY "${GIT_DIST_TMPDIR}")
  execute_process(COMMAND git archive --prefix "${GIT_DIST_NAME}/"
                  -o "${GIT_DIST_TMPDIR}/tmp.tar" "${GIT_RELEASE_TAG_NAME}"
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  execute_process(COMMAND tar -xf tmp.tar
                  WORKING_DIRECTORY "${GIT_DIST_TMPDIR}")
  file(REMOVE "${GIT_DIST_TMPDIR}/tmp.tar")

  git_release_version_from_git_tag("${GIT_RELEASE_TAG_NAME}" TRUE)

  git_release_version_to_file("${GIT_DIST_ROOT}/${GIT_VERSION_FILE}")
  if(GIT_VERSION_FILE_USE_COMPAT)
    git_release_version_to_file_compat("${GIT_DIST_ROOT}/${GIT_VERSION_FILE_COMPAT}")
  endif(GIT_VERSION_FILE_USE_COMPAT)
endfunction(git_archive_tree)

# Make a distribution of an arbitrary release.
#
# The same as git_dist, but this allows addition of any distribution
# rather than just the release in the current working tree.  This rule
# is intended for allowing retrospective addition of a project's
# entire release history (driven by a shell script), for example.
# See below for an example of how to do this.
#
# GIT_DIST_ROOT must be set to specify the release to distribute and
# GIT_RELEASE_VERSION must match the release version.  GIT_DIST_BRANCH may also
# require setting if not using the default.  GIT_RELEASE_TAG_NAME must
# be set to the tag name of the existing release.
function(git_dist)
  git_check_repo()

  execute_process(COMMAND git show-ref --tags -q ${GIT_DIST_TAG_NAME}
                  RESULT_VARIABLE show_ref_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (show_ref_status EQUAL 0)
    message(FATAL_ERROR "git dist tag ${GIT_DIST_TAG_NAME} already exists; not distributing")
  endif (show_ref_status EQUAL 0)

  if (NOT GIT_DIST_ENABLE)
    message(FATAL_ERROR "GIT_DIST_ENABLE not set; not distributing")
  endif (NOT GIT_DIST_ENABLE)

  message("Distributing ${CMAKE_PROJECT_NAME}-${GIT_RELEASE_VERSION} on git branch ${GIT_DIST_BRANCH}")
  set(dist_index "${GIT_DIST_TMPDIR}/index")
  if(EXISTS ${dist_index})
    file(REMOVE ${dist_index})
  endif(EXISTS ${dist_index})

  set(ENV{GIT_INDEX_FILE} ${dist_index})
  set(ENV{GIT_WORK_TREE} ${GIT_DIST_ROOT})
  execute_process(COMMAND git add -A
                  RESULT_VARIABLE git_add_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  unset(ENV{GIT_WORK_TREE})
  if(git_add_status GREATER 0)
    message(FATAL_ERROR "Failed to add archive tree to git index")
  endif(git_add_status GREATER 0)

  execute_process(COMMAND git write-tree
                  OUTPUT_VARIABLE tree
                  RESULT_VARIABLE git_write_tree_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  string(REPLACE "\n" "" tree "${tree}")
  if(git_tree_write_status GREATER 0)
    message(FATAL_ERROR "Failed to write git index to tree")
  endif(git_tree_write_status GREATER 0)

  unset(ENV{GIT_INDEX_FILE})

  file(REMOVE ${dist_index})

  string(LENGTH ${tree} tree_length)
  if (tree_length EQUAL 0)
    message(FATAL_ERROR "Failed to get tree hash")
  endif (tree_length EQUAL 0)

  git_rev_parse("${GIT_RELEASE_TAG_NAME}^{}" release_commit)

  set(commit_options -p ${release_commit})

  execute_process(COMMAND git show-ref --heads -s refs/heads/${GIT_DIST_BRANCH}
                  OUTPUT_VARIABLE dist_parent
                  RESULT_VARIABLE show_ref_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  string(REPLACE "\n" "" dist_parent "${dist_parent}")
  if (show_ref_status GREATER 0)
    set(newroot "(root-commit) ")
  else (show_ref_status GREATER 0)
    set(commit_options ${commit_options} -p ${dist_parent})
  endif (show_ref_status GREATER 0)

  file(WRITE "${GIT_DIST_TMPDIR}/commit-message" "${GIT_DIST_COMMIT_MESSAGE}")
  execute_process(COMMAND git commit-tree -F "${GIT_DIST_TMPDIR}/commit-message" "${tree}" ${commit_options}
                  OUTPUT_VARIABLE commit
                  RESULT_VARIABLE commit_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  file(REMOVE "${GIT_DIST_TMPDIR}/commit-message")
  string(REPLACE "\n" "" commit "${commit}")
  if (commit_status GREATER 0)
    message(FATAL_ERROR "Failed to commit tree")
  endif (commit_status GREATER 0)

  string(LENGTH ${commit} commit_length)
  if(${commit_length} EQUAL 0)
    message(FATAL_ERROR "Failed to get commit hash")
  endif(${commit_length} EQUAL 0)

  execute_process(COMMAND git update-ref "refs/heads/${GIT_DIST_BRANCH}" ${commit} ${dist_parent}
                  RESULT_VARIABLE update_ref_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if (update_ref_status GREATER 0)
    message(FATAL_ERROR "Failed to update reference for commit")
  endif (update_ref_status GREATER 0)

  message("[${GIT_DIST_BRANCH} ${newroot}${commit}] ${GIT_DIST_COMMIT_MESSAGE}")

  message("Creating git distribution tag ${GIT_DIST_TAG_NAME}")
  execute_process(COMMAND git tag -m "${GIT_DIST_TAG_MESSAGE}" ${GIT_DIST_TAG_OPTIONS} "${GIT_DIST_TAG_NAME}" ${commit}
                  RESULT_VARIABLE tag_status
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
  if(tag_status GREATER 0)
    message(FATAL_ERROR "Failed to create distribution tag ${GIT_DIST_TAG_NAME}: error status ${tag_status}")
  endif(tag_status GREATER 0)

  message(STATUS "${CMAKE_PROJECT_NAME} ${GIT_RELEASE_VERSION} distribution tagged as ${GIT_DIST_TAG_NAME}")
endfunction(git_dist)

# Add targets if not in script mode
if (NOT CMAKE_SCRIPT_MODE_FILE)

  set(script_options
    -D CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
    -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
    -D PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR})

  if (GIT_RELEASE_ENABLE OR GIT_DIST_ENABLE)
    # Preserve current configuration in a form which may be sourced by
    # a standalone script file.
    file(WRITE "${GIT_RELEASE_SETTINGS}"
"set(GIT_VERSION_FILE \"${GIT_VERSION_FILE}\")
set(GIT_VERSION_FILE_COMPAT \"${GIT_VERSION_FILE_COMPAT}\")
set(GIT_VERSION_FILE_USE_COMPAT \"${GIT_VERSION_FILE_USE_COMPAT}\")
set(GIT_RELEASE_VERSION \"${GIT_RELEASE_VERSION}\")
set(GIT_RELEASE_CHECK_UNCOMMITTED ${GIT_RELEASE_CHECK_UNCOMMITTED})
set(GIT_RELEASE_CHECK_UNTRACKED ${GIT_RELEASE_CHECK_UNTRACKED})
set(GIT_RELEASE_ENABLE ${GIT_RELEASE_ENABLE})
set(GIT_RELEASE_TAG_OPTIONS \"${GIT_RELEASE_TAG_OPTIONS}\")
set(GIT_RELEASE_TAG_NAME \"${GIT_RELEASE_TAG_NAME}\")
set(GIT_RELEASE_TAG_MATCH \"${GIT_RELEASE_TAG_MATCH}\")
set(GIT_RELEASE_TAG_REGEX \"${GIT_RELEASE_TAG_REGEX}\")
set(GIT_RELEASE_TAG_MESSAGE \"${GIT_RELEASE_TAG_MESSAGE}\")
set(GIT_DIST_ENABLE ${GIT_DIST_ENABLE})
set(GIT_DIST_TAG_OPTIONS \"${GIT_DIST_TAG_OPTIONS}\")
set(GIT_DIST_TAG_NAME \"${GIT_DIST_TAG_NAME}\")
set(GIT_DIST_TAG_MESSAGE \"${GIT_DIST_TAG_MESSAGE}\")
set(GIT_DIST_BRANCH \"${GIT_DIST_BRANCH}\")
set(GIT_DIST_COMMIT_MESSAGE \"${GIT_DIST_COMMIT_MESSAGE}\")
set(GIT_DIST_NAME \"${GIT_DIST_NAME}\")
set(GIT_DIST_TMPDIR \"${GIT_DIST_TMPDIR}\")
set(GIT_DIST_ROOT \"${GIT_DIST_ROOT}\")
")

  # The following targets re-execute this script file with the above
  # configuration.

    add_custom_target(git-check-repo
                      COMMAND ${CMAKE_COMMAND} ${script_options}
                              -D git_release_command=git-check-repo
                              -P ${CMAKE_CURRENT_LIST_FILE})
  endif (GIT_RELEASE_ENABLE OR GIT_DIST_ENABLE)

  if (GIT_RELEASE_ENABLE)
    add_custom_target(git-check-clean
                      COMMAND ${CMAKE_COMMAND} ${script_options}
                              -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                              -D git_release_command=git-check-clean
                              -P ${CMAKE_CURRENT_LIST_FILE})

    add_custom_target(git-release
                      COMMAND ${CMAKE_COMMAND} ${script_options}
                              -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                              -D git_release_command=git-release
                              -P ${CMAKE_CURRENT_LIST_FILE})
  endif (GIT_RELEASE_ENABLE)

  if (GIT_DIST_ENABLE)
    add_custom_target(git-distdir-archive
                      COMMAND ${CMAKE_COMMAND} ${script_options}
                              -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                              -D git_release_command=git-distdir-archive
                              -P ${CMAKE_CURRENT_LIST_FILE})

    add_custom_target(git-distdir DEPENDS git-distdir-archive)

    add_custom_target(git-dist
                      COMMAND ${CMAKE_COMMAND} ${script_options}
                              -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                              -D git_release_command=git-dist
                              -P ${CMAKE_CURRENT_LIST_FILE}
                      DEPENDS git-distdir)
  endif (GIT_DIST_ENABLE)
endif (NOT CMAKE_SCRIPT_MODE_FILE)

# In script mode, run specified command
if (CMAKE_SCRIPT_MODE_FILE)
  if(git_release_command)
    if(git_release_command STREQUAL git-check-repo)
      git_check_repo()
    endif(git_release_command STREQUAL git-check-repo)

    if(git_release_command STREQUAL git-check-clean)
      git_check_clean()
    endif(git_release_command STREQUAL git-check-clean)

    if(git_release_command STREQUAL git-release)
      git_release(ON)
    endif(git_release_command STREQUAL git-release)

    if(git_release_command STREQUAL git-distdir-archive)
      git_archive_tree()
    endif(git_release_command STREQUAL git-distdir-archive)

    if(git_release_command STREQUAL git-dist)
      git_dist()
    endif(git_release_command STREQUAL git-dist)
  endif(git_release_command)
endif (CMAKE_SCRIPT_MODE_FILE)

# Example: How to retrospectively insert the complete distribution
# history for a project.  Note: GIT_RELEASE_TAG_NAME must match the
# pattern used to tag all previous releases since this requires tags
# for all releases.
#
# #!/bin/sh
#
# set -e
#
# # Clean up any existing distribution branches and tags which could
# # interfere with addition of a complete clean distribution history.
# git tag -l | grep distribution | while read tag; do
#   git tag -d $tag
# done;
# git branch -l | grep distribution | while read branch; do
#   git branch -D $branch
# done;
#
# # Read an ordered list of versions from release-versions and get
# # distribution for each version from the given path and distribute
# # in git
# while read version; do
#   make git_dist_generic GIT_DIST_ROOT="/path/to/unpacked/releases/$package-$version" GIT_RELEASE_VERSION="$version" GIT_DIST_ENABLE=true
# done < release-versions

# Example: How to check that the added distributions are correctly
# representing the content of the old distributed releases following
# import as in the above example.
#
# #!/bin/sh
#
# set -e
#
# # For each version in the list of releases, check out and unpack
# # that version, and then compare it with the original.
# while read version; do
#   git checkout distribution/package-$version
#   rm -rf /tmp/package-$version
#   mkdir /tmp/package-$version
#   git archive HEAD | tar -x -C /tmp/$package-$version
#   diff -urN /tmp/$package-$version "/path/to/unpacked/releases/$package-$version" | lsdiff
#   rm -rf /tmp/package-$version
# done < release-versions
