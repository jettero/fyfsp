AC_PREREQ([2.69])

AC_INIT([fuck-you-focus-stealing-prevention], [1.0], [paul+fyfsp@voltar.org], [fyfsp], [https://github.com/jettero/fyfsp])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-xz])

AC_CONFIG_SRCDIR([.])
AC_CONFIG_HEADERS([config.h])

PKG_CHECK_MODULES(x11, x11, have_x11=yes, have_x11=no)

if test ! "x$have_x11" = xyes; then
    echo "this needs x11 libs and they weren't found for some reason";
    exit 1
fi

AC_SUBST(x11_CFLAGS)
AC_SUBST(x11_LIBS)

AC_CHECK_HEADERS([uthash.h])

AC_ARG_VAR([DEBUG_ALL], [turn on all debugging])

# NOTE: I execute these two for-loop-comments with a vim macro to generate the autoconf commands
# nmap gx :exe "r!" . substitute(getline('.'), '^[ ' . printf(&cms, '') . "]*", '', '')<cr>
# (course, I then have to edit the descriptions for the first for-loop result …)

# for i in $(egrep 'ifn?def DEBUG_' *.c | cut -d' ' -f2  | sort -u); do echo "AC_ARG_VAR([$i], [additional debugging])"; done
AC_ARG_VAR([DEBUG_ATOM_SETUP], [additional debugging — watch Atom (xcb/x11) instantiation])
AC_ARG_VAR([DEBUG_GET_PROPERTY], [additional debugging — watch XGetProperty])
AC_ARG_VAR([DEBUG_NO_SET_FOCUS], [additional debugging — disable XSetInputFocus calls])
AC_ARG_VAR([DEBUG_RECURSE_FOCUS_WINDOW_PROPERTIES], [additional debugging — force focus window properties traversal])
AC_ARG_VAR([DEBUG_RECURSE_MOUSE_WINDOW_PROPERTIES], [additional debugging — force mouse window properties traversal])
AC_ARG_VAR([DEBUG_TOP_WINDOW], [additional debugging — watch search for the root xwindow of a user window])
AC_ARG_VAR([DEBUG_CACHE], [additional debugging — watch various cache set/kill operations])

# for i in $(egrep 'ifn?def DEBUG_' *.c | cut -d' ' -f2  | sort -u); do echo "AS_IF([test \"x\$DEBUG_ALL\$$i\" = x],,DEBUG=1; AC_DEFINE([$i],,[additional debugging]))"; done
AS_IF([test "x$DEBUG_ALL$DEBUG_ATOM_SETUP" = x],,DEBUG=1; AC_DEFINE([DEBUG_ATOM_SETUP],,[additional debugging]))
AS_IF([test "x$DEBUG_ALL$DEBUG_CACHE" = x],,DEBUG=1; AC_DEFINE([DEBUG_CACHE],,[additional debugging]))
AS_IF([test "x$DEBUG_ALL$DEBUG_GET_PROPERTY" = x],,DEBUG=1; AC_DEFINE([DEBUG_GET_PROPERTY],,[additional debugging]))
AS_IF([test "x$DEBUG_ALL$DEBUG_RECURSE_FOCUS_WINDOW_PROPERTIES" = x],,DEBUG=1; AC_DEFINE([DEBUG_RECURSE_FOCUS_WINDOW_PROPERTIES],,[additional debugging]))
AS_IF([test "x$DEBUG_ALL$DEBUG_RECURSE_MOUSE_WINDOW_PROPERTIES" = x],,DEBUG=1; AC_DEFINE([DEBUG_RECURSE_MOUSE_WINDOW_PROPERTIES],,[additional debugging]))
AS_IF([test "x$DEBUG_ALL$DEBUG_TOP_WINDOW" = x],,DEBUG=1; AC_DEFINE([DEBUG_TOP_WINDOW],,[additional debugging]))

dnl have to specifically ask for this one …
AS_IF([test "x$DEBUG_NO_SET_FOCUS" = x],,DEBUG=1; AC_DEFINE([DEBUG_NO_SET_FOCUS],,[additional debugging]))

AC_ARG_VAR([DEBUG], [set to some non-empty string to enable debugging printf()s])
AS_IF([test "x$DEBUG" = x],,AC_DEFINE([DEBUG],,[debug mode enabled]))

## --with-upstart / --without-upstart #############################################################
AC_ARG_WITH([upstart],
  [AC_HELP_STRING([--with-upstart[[=LOCATION]]],
                  [install an upstart script that will automatically inflict this on upstart sessions.
                   (It may be better to install this by hand in ~/.config/upstart.)
                   If location is 'yes' or is omitted, it will default to the system upstart sessions directory.
                   ])])

if test ! "x$with_upstart" = x; then
    UPSTARTDIR="${datadir}/upstart/sessions"
    AC_SUBST(UPSTARTDIR)

else
    if test ! "x$withval" = x; then
        UPSTARTDIR="$withval"
        AC_SUBST(UPSTARTDIR)
    fi
fi

## --with-mouse-tracking / --without-mouse-tracking ###############################################
AC_ARG_WITH([mouse-tracking],
  [AC_HELP_STRING([--with-mouse-tracking],
                  [Track mouse xcrossover Enter/Leave events and
                   consider fixing focus on these events.
                   (Mostly, window managers do a much better job of this.)
                  ])])

AM_CONDITIONAL([MOUSE_TRACKING], [test ! "x$withval" = xno])
if test "x$withval" = xyes; then
    AC_DEFINE([MOUSE_TRACKING],,[enable mouse Enter/Leave tracking])
fi

## --with-tree-caching / --without-tree-caching ###############################################
AC_ARG_WITH([tree-caching],
  [AC_HELP_STRING([--with-tree-caching],
                  [Use the lightweight uthash.h toolkit as an X event/attribute cache.
                   It is not clear this will improve performance, but until we know better,
                   it is enabled by default.
                   (TODO: evaluate cache performance with a profiler)
                  ])])

AM_CONDITIONAL([CACHING], [test ! "x$withval" = xno])
if test "x$withval" = xno; then
    AC_DEFINE([NO_CACHING],,[fully disable the caching])
fi

## --with-debug-log / --without-debug-log ###############################################
AC_ARG_WITH([debug-log],
  [AC_HELP_STRING([--with-debug-log=[[LOCATION]]],
                  [build the binary with all sorts of print-to-file code so you
                   can figure out what went wrong if fyfsp does not do what
                   you expect. LOCATION will default to ~/.fyfsp.log (literally yours)
                   if it is omitted.
                   ])])

if test ! "x$with_debug_log" = x; then
    if test "x$withval" = xyes; then
        withval=~/.fyfsp.log
    fi
    AC_DEFINE([DEBUG],,[debug mode enabled])
    AC_DEFINE_UNQUOTED([DEBUG_LOG_TO_FILE],["$withval"],[spit out debug info here])
fi

# /--with

AM_CONDITIONAL([UPSTART], [test ! "x$with_upstart" = x])
AM_CONDITIONAL([CLANG], [test x$CC = "xclang"])

AC_PROG_CC
AC_PROG_INSTALL

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
