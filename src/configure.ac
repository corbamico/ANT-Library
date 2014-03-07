#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ([2.69])
AC_INIT(libantfs, 0.0.1, [github.com/corbamico/ANT-Library])
AC_CONFIG_SRCDIR([ANT_LIB/common/crc.h])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall])
AM_SILENT_RULES([yes])

# --enable-debug-log
AC_ARG_ENABLE([debug-log], [AS_HELP_STRING([--enable-debug-log],
	[enable debug logging (default n)])],
	[debug_log_enabled=$enableval],
	[debug_log_enabled='no'])
if test "x$debug_log_enabled" != "xno"; then
	AC_DEFINE([ENABLE_DEBUG_LOGGING], 1, [Debug message logging])
fi


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(usb-1.0, libusb_init)


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h paths.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime getcwd gettimeofday memset])

AC_OUTPUT(Makefile ANT_LIB/Makefile DEMO_ANTFS/Makefile DEMO_HRM/Makefile)
