#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Libintense-aep, [0.1.0], cartesianprogramming-AT-gmail-DOT-com)
AC_CONFIG_AUX_DIR([config])
# AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([include/intense-aep/intense_aep_config.hpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT

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
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([socket], [socket])

# Checks for header files.

# Checks for library functions.

saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -L$prefix/lib -lintense-io -lintense"
CXXFLAGS="$CSSFLAGS -I$prefix/include"
AC_TRY_LINK(
  [#include <intense.hpp>],
  [intense::Context context;],,
  AC_MSG_ERROR([** Require libintense with library version >= 1.0.0]),
)
LIBS="$saved_LIBS"
CXXFLAGS="$saved_CXXFLAGS"

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_CONFIG_FILES([Makefile tests/Makefile include/Makefile src/Makefile])

AC_OUTPUT
