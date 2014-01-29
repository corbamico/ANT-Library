#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(libant, 0.0.1, [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([ANT_LIB/software/USB/devices/usb_device.cpp])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(libant,0.0.1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_kill,[],
[
  AC_MSG_ERROR([Could  not find pthread library])
]) 

AC_CHECK_LIB(usb-1.0, libusb_init,[],
[
  AC_MSG_ERROR([Could  not find libusb-1.0 library])
]) 

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h paths.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime getcwd gettimeofday memset])

#AC_ARG_ENABLE(debug,
#AS_HELP_STRING([--enable-debug],
               #[enable debugging, default: no]),
#[case "${enableval}" in
             #yes) debug=true ;;
             #no)  debug=false ;;
             #*)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
#esac],
#[debug=false])
#AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_CONFIG_FILES([
 Makefile
 ANT_LIB/Makefile
 ANT_DLL/Makefile
 DEMO_LIB/Makefile
])
AC_OUTPUT
