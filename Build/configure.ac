#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([libNetwork], [1.0], [ardrone@ardrone.org])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(m4)

# automake initialisation and check for minimal automake API version 1.9
AM_INIT_AUTOMAKE ([foreign -Wall -Werror])

# use the C compiler for the following checks
AC_LANG([C])

##########################################################################
# Debug compilation support
##########################################################################


##########################################################################
# Define includes directories
##########################################################################
CFLAGS="$CFLAGS -I../Includes"

if test x"$INCLUDEDIR" != x""; then
    CFLAGS="$CFLAGS -I$INCLUDEDIR"
fi

CPPFLAGS="$CFLAGS"
##########################################################################
# Check headers
##########################################################################

# Checks for libSAL.h
AC_CHECK_HEADERS([libSAL/print.h libSAL/thread.h],,
	AC_MSG_ERROR(The libSAL header is required in order to build the library ! 
- Use INCLUDEDIR for indicate a specific include path.))

##########################################################################
# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# distribute additional compiler and linker flags
# --> set these variables instead of CFLAGS or LDFLAGS
AC_SUBST([CFLAGS])
AC_SUBST([LDFLAGS])

# Checks for library functions.


# Generates Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
"
