AC_INIT([plux], [0.1.0], [pekdon@gmail.com])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_SED

dnl Check for /usr/xpg4/bin/sh and use if available to make shipped scripts
dnl work as expected under Solaris
AC_CHECK_PROG([SH], [sh], [/usr/xpg4/bin/sh], [/bin/sh], [/usr/xpg4/bin])

AC_CHECK_HEADER([pty.h],
		[AC_DEFINE([HAVE_PTY_H], [1],
			   [Define to 1 if pty.h is available])])
AC_CHECK_HEADER([util.h],
		[AC_DEFINE([HAVE_UTIL_H], [1],
			   [Define to 1 if util.h is available])])
AC_CHECK_HEADER([libutil.h],
		[AC_DEFINE([HAVE_LIBUTIL_H], [1],
			   [Define to 1 if libutil.h is available])])
AC_CHECK_HEADER([termios.h],
		[AC_DEFINE([HAVE_TERMIOS_H], [1],
			   [Define to 1 if termios.h is available])])

AC_CHECK_LIB([util], [forkpty],
	     [LDFLAGS="$LDFLAGS -lutil"
	      HAVE_FORKPTY=yes],
	     [HAVE_FORKPTY=no])
if test "x$HAVE_FORKPTY" = "xno"; then
	AC_CHECK_FUNC(forkpty, [HAVE_FORKPTY=yes], [HAVE_FORKPTY=no])
fi
if test "x$HAVE_FORKPTY" = "xyes"; then
	AC_DEFINE([HAVE_FORKPTY], [1], [Define to 1 if forkpty is available])
fi

AC_CHECK_FUNC(setenv, [AC_DEFINE([HAVE_SETENV], [1],
				 [Define to 1 if setenv is available])])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
		 src/Makefile
		 stdlib/Makefile
		 test/Makefile])
AC_CONFIG_FILES([stdlib/gen_stdlib_builtins.sh],
		[chmod +x stdlib/gen_stdlib_builtins.sh])

CXXFLAGS="$CXXFLAGS -DSH=\\\"$SH\\\" -DPLUX_STDLIB_PATH=\\\"$datadir/plux/stdlib\\\""

AC_OUTPUT
