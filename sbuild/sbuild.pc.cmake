prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: Sbuild
Description: Debian Source Builder
Version: ${GIT_RELEASE_VERSION}
Libs: -L${libdir} -lsbuild
Cflags: -I${includedir}
