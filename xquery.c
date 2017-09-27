#include "config.h"
#include "xquery.h"
#include "window.h"
#include "cache.h"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <assert.h>

#define ATOMS 28
char *atom_str[] = {
    "_NET_WM_WINDOW_TYPE",
    "_NET_WM_WINDOW_TYPE_DESKTOP",
    "_NET_WM_WINDOW_TYPE_DOCK",
    "_NET_WM_WINDOW_TYPE_TOOLBAR",
    "_NET_WM_WINDOW_TYPE_MENU",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_NET_WM_WINDOW_TYPE_SPLASH",
    "_NET_WM_WINDOW_TYPE_DIALOG",
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    "_NET_WM_WINDOW_TYPE_TOOLTIP",
    "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    "_NET_WM_WINDOW_TYPE_COMBO",
    "_NET_WM_WINDOW_TYPE_DND",
    "_NET_WM_WINDOW_TYPE_NORMAL",

    "_NET_WM_STATE",
    "_NET_WM_STATE_MODAL",
    "_NET_WM_STATE_STICKY",
    "_NET_WM_STATE_MAXIMIZED_VERT",
    "_NET_WM_STATE_MAXIMIZED_HORZ",
    "_NET_WM_STATE_SHADED",
    "_NET_WM_STATE_SKIP_TASKBAR",
    "_NET_WM_STATE_SKIP_PAGER",
    "_NET_WM_STATE_HIDDEN",
    "_NET_WM_STATE_FULLSCREEN",
    "_NET_WM_STATE_ABOVE",
    "_NET_WM_STATE_BELOW",
    "_NET_WM_STATE_DEMANDS_ATTENTION"
};

Atom atom[ATOMS];

void window_query_setup(Display *display) {
    // FIXME: this should use XInternAtoms(); which does this heaving lifting for us
    int i;
    for (i = 0; i < ATOMS; i++) {
        atom[i] = XInternAtom(display, atom_str[i], False);
#ifdef DEBUG_ATOM_SETUP
        printf("populated atom_str[%2d] = %-38s ==> %ld\n", i, atom_str[i], atom[i]);
#endif
    }
}

bool is_this_atom( int xpti_or_xpsi, Atom a ) {
    if( xpti_or_xpsi < 0 || xpti_or_xpsi >= ATOMS )
        return false;

    return atom[xpti_or_xpsi] == a;
}

#ifdef  DEBUG_GET_PROPERTY
#ifndef DEBUG_LOG_TO_FILE
void dump_properties(int nitems, int actual_format, const unsigned char *prop) {
    int i,j;
    int bytes = actual_format / 8;

    for(i=0; i<nitems; i++) {
        if(i) printf(" |");

        // XXX: fix this with malloc(mah-string)
        // or be content that it never prints to the logfile
        for(j=0; j<bytes; j++)
            printf(" %02x", prop[bytes*i + j]);
    }

    printf("\n");
}
#endif
#endif

#define XPTI_MODE 0
#define XPSI_MODE 1

Bool XFetchClass(Display *display, Window window, char **class_name_return) {
    XClassHint *xch = XAllocClassHint();
    *class_name_return = NULL;
    if( xch ) {
        XGetClassHint(display, window, xch);
        if( xch->res_class )
            XFree(xch->res_class);
        if( xch->res_name )
            *class_name_return = xch->res_name;
        XFree(xch);
    }
    return 0;
}

int get_property(Display *display, Window window, unsigned char mode) {
    Atom actual_type;
    int actual_format, i,j;
    unsigned char *prop;
    unsigned long bytes_after, nitems;

    int loop_start, loop_end;
    Atom query;

    int retval = 0;

    if( mode ) { loop_start = XPSI_QUERY+1; loop_end = XPSI_END;   query = atom[XPSI_QUERY]; }
    else       { loop_start = XPTI_QUERY+1; loop_end = XPSI_QUERY; query = atom[XPTI_QUERY]; }

    Status status = XGetWindowProperty(display, window, query, 0, (~0L),
        False, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

    if (status != Success) {
        text_error("XGetWindowProperty(query=%ld, window=0x%lx) error: %s\n",
                query, window, Status2STR(status));

        return retval;
    }

#ifdef DEBUG_GET_PROPERTY
    printf("*** get_property(query=%ld, window=0x%lx) <nitems=%ld,type=%ld,format=%d>\n",
            query, window, nitems, actual_type, actual_format);
#endif

    if( !nitems || !actual_format )
        return retval;

#ifdef DEBUG_GET_PROPERTY
    dump_properties(nitems, actual_format, prop);
#endif

    // If the returned format is 8, the returned data is represented as a char
    // array.  If the returned format is 16, the returned data is represented
    // as a short array and should be cast to that type to obtain the elements.
    // If the returned format is 32, the returned data is represented as a long
    // array and should be cast to that type to obtain the elements.

    // This is assumed in the iteration below
    assert(actual_format == 32);

    Atom converted_property = 0;
    for(i=0; i < nitems; i++) {
        if ( (converted_property = ((unsigned long*) prop)[i]) ) {
            for (j = loop_start; j < loop_end; j++) {
                if (atom[j] == converted_property) {
                    retval |= mode ? XPSI2XPSV(j) : XPTI2XPTV(j);

                    // NOTE: It's really super handy to see property results
                    // so this doesn't get DEBUG_GET_PROPERTY … DEBUG is enough
                    printf("get_property(query=%ld, window=0x%lx) prop=%s retval=%d\n",
                            query, window, atom_str[j], retval);

                    break;
                }
            }
        }
    }

    XFree(prop);

    return retval;
}

xptv get_wm_type(Display *display, Window window) {
    return get_property(display, window, XPTI_MODE);
}

struct {
    xptv type;
    xpsv state;
    XID xid;
    Display *display;
} tmp;

bool _get_wm_type_callback(Window window) {
    tmp.type |= get_wm_type(tmp.display, window);
    return RECURSE_CONTINUE;
}

xptv get_wm_type_recursively(Display *display, Window window) {
    unsigned long retval;
    if( (retval = cache_window_long_look(window, CACHE_WM_TYPE_RECURSIVELY)) != NO_RESULT ) {
        printf("get_wm_type_recursively(0x%lx) [cached] %lu\n", window, retval);
        return (xptv) retval;
    }

    Window top_window = find_top_window(display, window);

    printf("get_wm_type_recursively(0x%lx→0x%lx)\n", window, top_window);

    tmp.type = 0;
    tmp.display = display;
    recurse_window_tree(display, top_window, _get_wm_type_callback);

    cache_window_long_value(display, window, tmp.type, CACHE_WM_TYPE_RECURSIVELY);

    return tmp.type;
}

xpsv get_wm_state(Display *display, Window window) {
    return get_property(display, window, XPSI_MODE);
}

bool _get_wm_state_callback(Window window) {
    tmp.state |= get_wm_state(tmp.display, window);
    return RECURSE_CONTINUE;
}

xpsv get_wm_state_recursively(Display *display, Window window) {
    unsigned long retval;
    if( (retval = cache_window_long_look(window, CACHE_WM_STATE_RECURSIVELY)) != NO_RESULT ) {
        printf("get_wm_state_recursively(0x%lx) [cached] %lu\n", window, retval);
        return (xpsv) retval;
    }

    Window top_window = find_top_window(display, window);

    printf("get_wm_state_recursively(0x%lx→0x%lx)\n", window, top_window);

    tmp.state = 0;
    tmp.display = display;
    recurse_window_tree(display, top_window, _get_wm_state_callback);

    cache_window_long_value(display, window, tmp.state, CACHE_WM_STATE_RECURSIVELY);

    return tmp.state;
}

bool _get_wm_group_callback(Window window) {
    XWMHints *hints = XGetWMHints(tmp.display, window);

    if( hints == NULL )
        return RECURSE_CONTINUE;

    tmp.xid = hints->window_group;
    XFree(hints);

    return RECURSE_FINISHED;
}

XID get_wm_group(Display *display, Window window) {
    XID retval;
    if( (retval = cache_window_long_look(window, CACHE_WM_GXID_RECURSIVELY)) != NO_RESULT ) {
        printf("get_wm_group(0x%lx) [cached] → 0x%lx\n", window, retval);
        return retval;
    }

    Window top_window = find_top_window(display, window);

    tmp.xid = 0;
    tmp.display = display;
    recurse_window_tree(display, top_window, _get_wm_group_callback);

    cache_window_long_value(display, window, tmp.xid, CACHE_WM_GXID_RECURSIVELY);

    printf("get_wm_group(0x%lx) → 0x%lx\n", window, tmp.xid);

    return tmp.xid;
}
