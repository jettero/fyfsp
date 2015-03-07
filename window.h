#ifndef WINDOW_H
#define WINDOW_H

#if !OBSESSIVE_QUERY_LOOP_PERIOD
#define OBSESSIVE_QUERY_LOOP_PERIOD 50000
#endif

#include <X11/Xlib.h>
#include "global-types.h"

Window find_rat_window(Display *display);
Window find_focus_window(Display *display);

bool test_rat_window_has_focus(int depth, Display *display, Window has_rat, Window has_focus);
void fuck_window_manager(Display *display, Window give_focus);

bool is_special_window(Display *display, Window window);
bool is_modal(Display *display, Window window);

Window find_top_window(Display *display, Window window);

#define find_normal_window_from_the_top(display, window) \
    (find_normal_window(display, find_top_window(display, window)))

bool recurse_window_tree(Display *display, Window window, bool (*callback)(Window) );

bool same_group(Display *display, Window one, Window two);

#define RECURSE_FINISHED true
#define RECURSE_CONTINUE false

#endif
