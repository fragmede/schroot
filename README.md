# schroot

Lightweight virtualisation tool

## Features

- Uses standard Unix chroot(2) to run programs in virtual environments
- Create named environments for fast and easy access
- Run a command or login shell in the virtual environment
- Mounts filesystems from the host in the virtual environment to
  provide seamless access to user data
- Change to another user or to root (uses PAM for authentication and
  authorisation, and like sudo can provide password authentication or
  passwordless access)
- Switch to a different system in the virtual environment, for example
  a 32-bit environment on a 64-bit host system
- Run entirely different architectures in the virtual environment, for
  example a PowerPC, ARM or MIPS platform on an amd64 host system--no
  need to cross-compile when you can run the host directly
- Snapshotting, including use of Btrfs and union filesystems, to
  provide clean copies of the virtual environment
- Session management--create persistent or temporary named copies of a
  virtual environment
- All the above may be done in parallel with multiple users

## Supported platforms

- FreeBSD 10.2
- Linux

## Build Dependencies

- An ISO C++11 compiler (Clang++ ≥ 3.4 or GCC ≥ 4.8 are recommended)
- The libraries and tools listed in the table below

Dependency   | Version | When required | Debian             | FreeBSD
-------------|---------|---------------|--------------------|------------------
C++ compiler | [C++11] | Build         | `build-essential`  | N/A (base system)
CMake        | ≥ 3.2.0 | Build         | `cmake`            | `cmake`
Boost        | ≥ 1.54  | Build, Run    | `libboost-all-dev` | `boost-all`
PAM          | ≥ 1.1.8 | Build, Run    | `libpam0g-dev`     | N/A (base system)
troff        |         | (Build) [git] | `groff`            | `groff`
gettext      | ≥ 0.16  | (Build, Run)  | `gettext`          | `gettext`
Doxygen      | ≥ 1.8   | (Build) [git] | `doxygen`          | `doxygen`
po4a         | ≥ 0.40  | (Build) [git] | `po4a`             | `po4a`
googletest   | ≥ 1.7   | (Build tests) | `libgtest-dev`     | `googletest`

Optional dependencies are enclosed with parentheses in the "when
required" column.

Note that the versions listed are the minimum versions being actively
tested and supported.  Older versions may work, or may work with minor
effort, but have not been tested.


## Building and installation

### Debian

Debian and derivative systems do not provide a precompiled googletest
library.  Build a version for the schroot build with, for example:

```
mkdir build
cd build
mkdir gtest
(
  cd gtest
  cmake /usr/src/gtest
  make VERBOSE=1
)
```

And then build with:

```
cmake -DGTEST_ROOT="$(pwd)/gtest" /path/to/schroot
make
fakeroot ctest -V
```

### FreeBSD

```
export CMAKE_PREFIX_PATH=/usr/local
cmake /path/to/schroot
make
fakeroot ctest -V
```


## Build customisation

After running CMake as above, run `cmake -LH` to see basic
configurable options.  The following basic options are supported:

- `btrfs-snapshot=(ON|OFF)` Enable support for btrfs snapshots (requires Btrfs)
- `debug=(ON|OFF)` Enable debugging messages
- `default_environment_filter=REGEX` Default environment filter
- `doxygen=(ON|OFF)` Enable doxygen documentation
- `loopback=(ON|OFF)` Enable support for loopback mounts
- `nls=(ON|OFF)` Enable national language support (requires gettext)
- `pam=(ON|OFF)` Enable support for PAM authentication (requires libpam)
- `personality=(ON|OFF)` Enable personality support (Linux only)
- `test=(ON|OFF)` Enable unit tests
- `union=(ON|OFF)` Enable support for union mounts
- `unshare=(ON|OFF)` Enable unshare support (Linux only)

CMake will autodetect and enable all available features by default, so
these options are mostly useful for disabling features which are not
required.

Run `cmake -LA` to see all settable options.  `CMAKE_INSTALL_PREFIX`
is the equivalent of the Autoconf `--prefix` option.  Additionally,
`CMAKE_INSTALL_SYSCONFDIR`, `CMAKE_INSTALL_LOCALSTATEDIR`,
`CMAKE_INSTALL_LIBDIR` etc. provide the equivalent `sysconfdir`,
`localstatedir` and `libdir` Autoconf options.  It is also possible to
set the distributor name with `-Ddistributor`.

Run `make doc` to make the doxygen documentation.
Run `ctest` to run the testsuite.

Note that the testsuite should be run under `fakeroot` or real root in
order to work correctly, due to it testing permissions from the
perspective of the root user with root-owned files.


## Configuration

See schroot.conf(5) and schroot-setup(5).


## Running

See schroot(1).


## Translation

If you would like to see the schroot messages output in your own
language, please consider translating the pot file (`po/schroot.pot`).
If you would like to see the schroot man pages in your own language,
please consider translating the pot file (`man/po/schroot-man.pot`).
