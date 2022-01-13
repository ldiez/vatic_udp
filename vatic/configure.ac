#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Vatic], [0.0.1], [ldiez@tlmat.unican.es])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE(-Wall -Werror foreign )
AC_CONFIG_SRCDIR([src/Vatic.h])
AC_CONFIG_HEADERS([config.h])

AC_LANG(C++)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# CXXFLAGS="-O3"

# Checks for libraries.
AC_CONFIG_MACRO_DIR([m4])
AX_BOOST_SYSTEM()
AC_CHECK_LIB([pthread], [pthread_mutex_init], [], [echo "pthreads not found.  Please install pthread library before proceeding"; exit -1])



# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fenv.h float.h inttypes.h limits.h locale.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T



# Checks for library functions.
AC_CHECK_FUNCS([inet_ntoa memset socket strerror])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
