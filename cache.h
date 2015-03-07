#ifndef CACHE_H
#define CACHE_H

#include "config.h"
#include "global-types.h"

#include <X11/Xlib.h>

#ifdef HAVE_UTHASH_H
#include <uthash.h>
#else
#include "contrib/uthash.h"
#endif

// I figure the odds of xptv and xpsv and even XID hitting
// max-long by accident are rather low — but I guess who knows
// we'll assert( cache_value != NO_RESULT ) to be sure.
#define NO_RESULT (~(0L))

#ifdef NO_CACHING
#define cache_window_long_value(...)
#define cache_kill_window(...)
#define cache_kill_window_state_related(...)
#define cache_window_long_look(...) NO_RESULT
#else
void cache_window_long_value(Display *display, Window window, unsigned long value, char value_code);
unsigned long cache_window_long_look(Window window, char value_code);

void cache_kill_window(Display *display, Window window);
void cache_kill_window_state_related(Display *display, Window window);
#endif

#define CACHE_WM_STATE_RECURSIVELY   'S'
#define CACHE_WM_TYPE_RECURSIVELY    'T'
#define CACHE_WM_GXID_RECURSIVELY    'G'

#define CACHE_FIND_NORMAL_WINDOW 'N'
#define CACHE_FIND_TOP_WINDOW    'R'

#ifdef DEBUG
#define CODE2STR(X) ( X=='S' ? "CACHE_WM_STATE_RECURSIVELY" \
                    : X=='T' ? "CACHE_WM_TYPE_RECURSIVELY" \
                    : X=='G' ? "CACHE_WM_GXID_RECURSIVELY" \
                    : X=='N' ? "CACHE_FIND_NORMAL_WINDOW" \
                    : X=='R' ? "CACHE_FIND_TOP_WINDOW" : "<unknown>" )
#endif

#endif
