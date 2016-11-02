AC_PREREQ([2.64])

m4_define([libpointing_major_version], [0])
m4_define([libpointing_minor_version], [0])
m4_define([libpointing_micro_version], [0])
m4_define([libpointing_version],
          [libpointing_major_version.libpointing_minor_version.libpointing_micro_version])

AC_INIT([libpointing],
        [libpointing_version],
        [https://github.com/INRIA/libpointing], 
        [libpointing],
        [https://github.com/INRIA/libpointing])

AC_SUBST([LIBPOINTING_VERSION_MAJOR], [libpointing_major_version])
AC_SUBST([LIBPOINTING_VERSION_MINOR], [libpointing_minor_version])
AC_SUBST([LIBPOINTING_VERSION_MICRO], [libpointing_micro_version])
AC_SUBST([LIBPOINTING_VERSION], [libpointing_version])

AC_DEFINE([LIBPOINTING_VERSION_MAJOR], [libpointing_major_version], "libpointing major version number")
AC_DEFINE([LIBPOINTING_VERSION_MINOR], [libpointing_minor_version], "libpointing minor version number")
AC_DEFINE([LIBPOINTING_VERSION_MICRO], [libpointing_micro_version], "libpointing micro version number")

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz subdir-objects])

AM_SILENT_RULES([yes])
AC_USE_SYSTEM_EXTENSIONS

# Check for programs
AC_PROG_CXX

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES(LIBUDEV, [libudev])
PKG_CHECK_MODULES(LIBXRANDR, [xrandr])
PKG_CHECK_MODULES(LIBXI, [xi])
PKG_CHECK_MODULES(LIBX11, [x11])

if test "x$GCC" = "xyes"; then
	GCC_CXXFLAGS="-Wall -Wextra -Wno-unused-parameter -g -fvisibility=hidden"
	GCC_CFLAGS="$GCC_CXXFLAGS -Wmissing-prototypes -Wstrict-prototypes"
fi
AC_SUBST(GCC_CFLAGS)
AC_SUBST(GCC_CXXFLAGS)

AC_CONFIG_FILES([Makefile
                 libpointing.pc])
AC_OUTPUT
