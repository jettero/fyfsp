#include "config.h"
#include "window.h"
#include "xquery.h"
#include "cache.h"

#include <unistd.h>

Window find_rat_window(Display *display) {
    int num_screen = ScreenCount(display);
    Window root, child;
    int blah, i;

    do {
        for (i = 0; i < num_screen; i++)
            if (XQueryPointer(display, RootWindow(display, i), &root, &child, &blah, &blah, &blah, &blah,
                    (unsigned int *) &blah))
                break;

        usleep(OBSESSIVE_QUERY_LOOP_PERIOD);

    } while (!child);

    return child;
}

Window find_focus_window(Display *display) {
    Window child;
    int blah, i;

    do {
        XGetInputFocus(display, &child, &blah);
        usleep(OBSESSIVE_QUERY_LOOP_PERIOD);

    } while (!child);

    return child;
}

Window find_top_window(Display *display, Window window) {
    Window retval;
    if( (retval = cache_window_long_look(window, CACHE_FIND_TOP_WINDOW)) != NO_RESULT ) {
#ifdef DEBUG_TOP_WINDOW
        printf("find_top_window(0x%lx) [cached] 0x%lx\n", window, retval);
#endif
        return retval;
    }

    retval = window;

    Window root, parent, *children = NULL;
    unsigned int nchild;

    while(1) {
        Status status = XQueryTree(display, retval, &root, &parent, &children, &nchild);
        // XQueryTree(3) claims it can return BadWindow, which seems spurious,
        // since it *really* returns true false (ie, worked or didn't).
        // Not sure how to check BadWindow, if it really sets that at all …

#ifdef DEBUG_TOP_WINDOW
        printf("find_top_window(0x%lx→0x%lx) root=0x%lx parent=0x%lx [status=%d] children=%p nchild=%d\n",
                window, retval, root, parent, status, children, nchild);
#endif

        if( !status )
            break;

        if( children != NULL )
            XFree(children);

        if( parent == root )
            break;

        retval = parent;
    }

#ifdef DEBUG_TOP_WINDOW
    printf(" ← ftw 0x%lx\n", retval);
#endif

    cache_window_long_value(display, window, retval, CACHE_FIND_TOP_WINDOW);

    return retval;
}

struct {
    Window window;
    Display *display;
} tmp;

bool _find_normal_window(Window window) {
    if( get_wm_type(tmp.display, window) & XPTV_NORMAL ) {
        tmp.window = window;
        return RECURSE_FINISHED;
    }

    return RECURSE_CONTINUE;
}

Window find_normal_window(Display *display, Window window) {
    Window retval;
    if( (retval = cache_window_long_look(window, CACHE_FIND_NORMAL_WINDOW)) != NO_RESULT )
        return retval;

    tmp.window = window;
    tmp.display = display;

    recurse_window_tree(display, window, _find_normal_window);

    cache_window_long_value(display, window, tmp.window, CACHE_FIND_NORMAL_WINDOW);

    return tmp.window;
}

bool recurse_window_tree(Display *display, Window window, bool (*callback)(Window) ) {
    Window root, parent, *children;
    unsigned int nchild, i;
    bool result = false;

    if( callback(window) )
        return true;

    Status status = XQueryTree(display, window, &root, &parent, &children, &nchild);

    if( !status )
        return result;

    if( children != NULL ) {
        int i;
        for(i=0; i<nchild; i++)
            if( (result = recurse_window_tree(display, children[i], callback)) )
                break;

        XFree(children);
    }

    return result;
}

bool test_rat_window_has_focus(int depth, Display *display, Window has_rat, Window has_focus) {

    if (has_rat == has_focus) {
        printf("test_rat_window_has_focus(0x%lx, 0x%lx) = true\n", has_rat, has_focus);

        return true;
    }

    Window top_rat   = find_top_window( display, has_rat   );
    Window top_focus = find_top_window( display, has_focus );

    if ( top_rat == top_focus ) {
        printf("test_rat_window_has_focus(0x%lx, 0x%lx) ≡ [top](0x%lx, 0x%lx) = true\n",
                has_rat, has_focus, top_rat, top_focus);

        return true;
    }

    return false;
}

bool fuck_window_manager(Display *display, Window give_focus) {
    Window target = find_normal_window_from_the_top(display, give_focus);
    bool did_something = false;

    if( !target ) {
        text_error("couldn't locate normal window for give-focus(0x%lx) target\n", give_focus);
        return did_something;
    }

#ifdef DEBUG_NO_SET_FOCUS
#define XSetInputFocus(...) 0xbeef
#endif

    Status status = XSetInputFocus(display, target, RevertToNone, CurrentTime);

    switch( status ) {
        case BadMatch:
        case BadValue:
        case BadWindow:
            text_error("XsetInputFocus(0x%lx→0x%lx) error(%d): %s\n",
                    give_focus, target, status, Status2STR(status) );
            break;

        case 0xbeef:
            printf("[XXX Disabled]");
            /* no break */

        default:
            printf("XSetInputFocus(0x%lx→0x%lx) → success!! \\o/\n", give_focus, target);
            did_something = 1;
    }

    return did_something;
}

bool is_special_window(Display *display, Window window) {
    // NOTE: tempting to put the function calls right in the
    // function-alike-macro-call do not do this.  you'll end up calling the
    // function for each CONSTANT use in the macro.

    xptv type  = get_wm_type_recursively(display, window);
    xpsv state = get_wm_state_recursively(display, window);

    if( TYPE_ISA_SPECIAL(type) )
        return true;

    if( STATE_ISA_SPECIAL(state) )
        return true;

    return false;
}

bool is_modal(Display *display, Window window) {
    if( get_wm_state_recursively(display, window) & XPSV_MODAL )
        return true;

    return false;
}

bool same_group(Display *display, Window one, Window two) {
    XID g1 = get_wm_group(display, one);
    XID g2;

    if( g1 == -1 ) {
        printf("same_group(0x%lx, 0x%lx) → NULL result\n", one, two);
        return false;
    }

    if( g1 == (g2 = get_wm_group(display, two)) ) {
        printf("same_group(0x%lx, 0x%lx) → 0x%lx true\n", one, two, g1);
        return true;
    }

    printf("same_group(0x%lx, 0x%lx) → 0x%lx / 0x%lx false\n", one, two, g1, g2);
    return false;
}
