#include "config.h"

#include <X11/Xlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "global-types.h"
#include "window.h"
#include "xquery.h"
#include "cache.h"

char *filter = NULL;

void process_options(int argc, char **argv) {
    int c;
    while( (c=getopt(argc, argv, "hf:")) != -1 ) {
        switch(c) {
            case 'h':
                dprintf(1, "fyfsp [-h] [-f filter]\n");
                dprintf(1, "\nFiltering is simple string matching against the window title and class. "
                        "Fancier filters are a todo item. If the filter string is in either the title "
                        "or in the class: go ahead and fyfsp the focus change, otherwise, ignore it.\n"
                        );
                exit(0);
            case 'f':
                filter = optarg;
                break;
        }
    }
}

int (*defaulthandler)();

int errorhandler(Display *display, XErrorEvent *error) {
    // I tend to print the errors locally with more information
    //  (well, at least a hint about which call caused the problem)
    //
    // fprintf(stderr, "XerrorEvent(%d): \n", error->error_code);

    // The default exits.  I probably never want that to happen.
    //
    // if (error->error_code != BadWindow)
    //     (*defaulthandler)(display, error);

    return 0;
}

void exit_gracefully(int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
        printf("normal exit\n");
        exit(0);
    }

    printf("unexpected signal\n");
    exit(1);
}

void do_mainloop_checks(Display *display, Window has_rat, Window has_focus) {
#ifdef DEBUG_RECURSE_MOUSE_WINDOW_PROPERTIES
    get_wm_type_recursively(display, has_rat);
    get_wm_state_recursively(display, has_rat);
#endif

#ifdef DEBUG_RECURSE_FOCUS_WINDOW_PROPERTIES
    get_wm_type_recursively(display, has_focus);
    get_wm_state_recursively(display, has_focus);
#endif

    if (test_rat_window_has_focus(0, display, has_rat, has_focus))
        printf("has-focus: 0x%lx ∈ has-rat: 0x%lx\n", has_focus, has_rat);

    else {
        if (is_special_window(display, has_rat))
            printf("has-focus: 0x%lx ∅ has-rat: 0x%lx\n", has_focus, has_rat);

        else {
            if( is_modal(display, has_focus) && same_group(display, has_focus, has_rat) )
                printf("has-focus: 0x%lx « has-rat: 0x%lx\n", has_focus, has_rat);

            else {
                printf("has-focus: 0x%lx ∉ has-rat: 0x%lx\n", has_focus, has_rat);

                int skip = 0;
                if( filter ) {
                    int name_match = 0;
                    int class_match = 0;

                    char *wm_name;
                    char *wm_class;

                    XFetchName(display, has_rat, &wm_name);
                    XFetchClass(display, has_rat, &wm_class);

                    printf("filter=\"%s\"", filter);

                    if(wm_name) {
                        name_match = strstr(wm_name, filter) != NULL;
                        printf(" wm_name=\"%s\"<%d>", wm_name, name_match);
                        XFree(wm_name);
                    }

                    if(wm_class) {
                        class_match = strstr(wm_class, filter) != NULL;
                        printf(" wm_class=\"%s\"<%d>", wm_class, class_match);
                        XFree(wm_class);
                    }

                    skip = !(name_match || class_match);
                    printf(" skip=%d\n", skip);
                }

                if( !skip )
                    fuck_window_manager(display, has_rat);
            }
        }
    }
}

int main(int argc, char **argv) {
    process_options(argc,argv);

    Display *display = XOpenDisplay(NULL);
    XEvent event;
    int num_screen,i;

#ifdef DEBUG
    char *_changed_property_atom_name;
#endif

    if (signal(SIGINT, exit_gracefully) == SIG_ERR)
        perror("error setting SIGINT handler");
    // exit(??) :- why? just explain that we can't catch it and ignore it

#ifdef DEBUG_LOG_TO_FILE
    if( fclose(stdout) ) {
        perror("couldn't close stdout for DEBUG_LOG_TO_FILE");
        exit(1);
    }

    if( (stdout = fopen(DEBUG_LOG_TO_FILE, "a")) == NULL ) {
        perror("couldn't open DEBUG_LOG_TO_FILE for write");
        exit(1);
    }

    fprintf(stderr, "debugging output is being directed to %s\n", DEBUG_LOG_TO_FILE);
#endif

    setlinebuf(stdout);

    if (!display) {
        perror("could not open display");
        exit(1);
    }

    defaulthandler = XSetErrorHandler(errorhandler);

    window_query_setup(display);

	defaulthandler = XSetErrorHandler(errorhandler);
	num_screen = ScreenCount(display);
	for (i = 0; i < num_screen; i++)
		XSelectInput(display, RootWindow(display, i), SubstructureNotifyMask);

    Window has_rat = find_rat_window(display);
    Window has_focus = find_focus_window(display);

#define xsi(window) XSelectInput(display, window, EnterWindowMask|LeaveWindowMask|FocusChangeMask)
    xsi(has_rat);
    xsi(has_focus);

#define find_rat_and_focus_and_handle(X) do { \
    xsi(has_rat = find_rat_window(display)); \
    xsi(has_focus = find_focus_window(display)); \
    xsi(X); \
} while(0)

    while (1) {
		XNextEvent(display, &event);

        switch (event.type) {
            // Main Loop Events:
            case MapNotify:
                printf("XEvent(MapNotify) 0x%lx\n", event.xmap.window);
                find_rat_and_focus_and_handle( event.xmap.window );
                do_mainloop_checks(display, has_rat, has_focus);
                break;

            case FocusIn:
            case FocusOut:
                printf("XEvent(FocusOut) 0x%lx\n", event.xfocus.window);
                find_rat_and_focus_and_handle( event.xfocus.window );
                do_mainloop_checks(display, has_rat, has_focus);
                break;

#ifdef TRACK_MOUSE_MOVEMENTS
            case EnterNotify:
            case LeaveNotify:
                printf("XEvent(LeaveNotify) 0x%lx\n", event.xcrossing.window);
                find_rat_and_focus_and_handle( event.xcrossing.window );
                do_mainloop_checks(display, has_rat, has_focus);
                break;

            case MotionNotify:
                // we don't actually ask for or receive this, but we might some day
                printf("XEvent(MotionNotify) 0x%lx\n", event.xmotion.window);
                find_rat_and_focus_and_handle( event.xmotion.window );
                do_mainloop_checks(display, has_rat, has_focus);
                break;
#endif

            // Cache Events:
            case PropertyNotify: // ← must set PropertyChangeMask set in cache-add functions
#ifdef DEBUG
                _changed_property_atom_name = XGetAtomName(display, event.xproperty.atom);
                printf("XEvent(PropertyChangeNotify) 0x%lx %s\n",
                    event.xproperty.window, _changed_property_atom_name);

                if( _changed_property_atom_name != NULL )
                    XFree(_changed_property_atom_name);
#endif

                if( is_this_atom(XPSI_WM_STATE, event.xproperty.atom) )
                    cache_kill_window_state_related(display, event.xproperty.window);

                break;

            case ReparentNotify: // (SubstructureMask)
                printf("XEvent(ReparentNotify) 0x%lx\n", event.xreparent.window);
                cache_kill_window(display, event.xreparent.window);
                break;

            case UnmapNotify: // (SubstructureMask)
                printf("XEvent(UnmapNotify) 0x%lx\n", event.xunmap.window);
                cache_kill_window(display, event.xunmap.window);
                break;
        }
    }

    XCloseDisplay(display);

    return 0;
}

#ifdef DEBUG_LOG_TO_FILE
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIME_STRING_BUFFER 25
char buffer[TIME_STRING_BUFFER];
char *current_time_string() {
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);

    snprintf(buffer,  7, "%-5d ", getpid());
    strftime(buffer + 6, TIME_STRING_BUFFER, "%Y-%m-%d %H:%M:%S", loctime);

    return buffer;
}
#endif
