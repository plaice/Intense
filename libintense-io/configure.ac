#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(Libintense-io, [0.1.0], cartesianprogramming-AT-gmail-DOT-com)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([include/intense-io/intense_io_config.hpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT

AH_TEMPLATE([BIG_ENDIAN_SYSTEM], [])
AH_TEMPLATE([HAVE_PTHREAD_MUTEX_ERRORCHECK], [])
AH_TEMPLATE([HAVE_XDR_HYPER], [])
AH_TEMPLATE([HAVE_XDR_U_HYPER], [])
AH_TEMPLATE([LITTLE_ENDIAN_SYSTEM], [])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lfl':
AC_CHECK_LIB([fl], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([nsl], [xdrmem_create],
             AC_SUBST(XDR_LIBS, [-lnsl]), AC_SUBST(XDR_LIBS, []))

saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -lpthread"
AC_TRY_LINK(
  [#include <pthread.h>],
  [pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);],
  AC_DEFINE(HAVE_PTHREAD_MUTEX_ERRORCHECK),
  AC_MSG_WARN([  ** NO PTHREAD_MUTEX_ERRORCHECK found on this system...]),
)
LIBS="$saved_LIBS"

if test "$ac_cv_lib_nsl_xdrmem_create" = yes; then
saved_LIBS="$LIBS"
saved_CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS -lnsl"
fi
AC_TRY_LINK(
  [#include <rpc/types.h>
#include <rpc/xdr.h>],
  [XDR encoder;
   XDR decoder;
   const static int bufferSize = 8;
   char outBuffer[8];
   int testInt = 1;
   xdrmem_create(&encoder, outBuffer, bufferSize, XDR_ENCODE);
   xdr_int(&encoder, &testInt);],,
  AC_MSG_ERROR([  ** NO WORKING <rpc/xdr.h> found on this system...]),
)
if test "$ac_cv_lib_nsl" = yes; then
LIBS="$saved_LIBS"
fi

if test "$ac_cv_lib_nsl_xdrmem_create" = yes; then
AC_CHECK_LIB([nsl], [xdr_hyper], AC_DEFINE(HAVE_XDR_HYPER),)
else
AC_CHECK_FUNC([xdr_hyper], AC_DEFINE(HAVE_XDR_HYPER),)
fi


if test "$ac_cv_lib_nsl_xdrmem_create" = yes; then
AC_CHECK_LIB([nsl], [xdr_u_hyper], AC_DEFINE(HAVE_XDR_U_HYPER),)
else
AC_CHECK_FUNC([xdr_u_hyper], AC_DEFINE(HAVE_XDR_U_HYPER),)
fi


AC_TRY_RUN([
int main()
{
	short s = 1;
	short* ptr = &s;
	unsigned char c = *((char*)ptr);
	return c;	
}],
AC_DEFINE(BIG_ENDIAN_SYSTEM), AC_DEFINE(LITTLE_ENDIAN_SYSTEM))

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 tests/Makefile
                 include/Makefile
                 src/Makefile])
AC_OUTPUT
