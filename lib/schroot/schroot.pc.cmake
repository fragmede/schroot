prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: schroot
Description: chroot maintenance and session manager
Version: ${GIT_RELEASE_VERSION}
Libs: -L${libdir} -lschroot
Cflags: -I${includedir}
