#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT(Libintense, [0.1.0], gnu-AT-hackomatic-DOT-net)
AM_INIT_AUTOMAKE
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([include/intense/intense_config.hpp])

AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_LIBTOOL
#AC_LIBTOOL_CXX

if test "$prefix" != NONE; then
AC_SUBST(INCLUDE_PREFIX, "-I$prefix/include")
AC_SUBST(LINK_PREFIX, "-L$prefix/lib")
else
AC_SUBST(INCLUDE_PREFIX, "")
AC_SUBST(LINK_PREFIX, "")
fi

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lfl':
AC_CHECK_LIB([fl], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])

saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -L$prefix/lib -lintense-io"
CXXFLAGS="$CSSFLAGS -I$prefix/include"
AC_TRY_LINK(
  [#include <sstream>
   #include <intense-io.hpp>],
  [intense::io::XDRBaseSerialiser serialiser;
   std::ostringstream oss;
   serialiser.out(1, oss);],,
  AC_MSG_ERROR([** Require libintense-io with library version >= 1.0.0]),
)
LIBS="$saved_LIBS"
CXXFLAGS="$saved_CXXFLAGS"

saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -lpthread"
AC_TRY_LINK(
  [#include <pthread.h>],
  [pthread_spinlock_t lock;
   pthread_spin_init(&lock, 0);],AC_DEFINE(DEFINITELY_HAVE_POSIX_SPINLOCKS),
  AC_MSG_WARN([  ** NO pthread spinlocks found on this system...]),
)
LIBS="$saved_LIBS"

saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -lpthread"
AC_TRY_LINK(
  [#include <pthread.h>],
  [pthread_mutex_t mutex;
   pthread_mutex_init(&mutex, 0);],AC_DEFINE(DEFINITELY_HAVE_POSIX_MUTEXES),
  AC_MSG_WARN([  ** NO pthread mutexes found on this system...]),
)
LIBS="$saved_LIBS"

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 include/Makefile
                 src/Makefile])
AC_OUTPUT
