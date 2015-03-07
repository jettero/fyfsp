#ifndef XPROPERTY_QUERY_H
#define XPROPERTY_QUERY_H

#include "global-types.h"

typedef enum {
    XPTI_WM_WINDOW_TYPE        =  0,

    XPTI_DESKTOP               =  1,
    XPTI_DOCK                  =  2,
    XPTI_TOOLBAR               =  3,
    XPTI_MENU                  =  4,
    XPTI_UTILITY               =  5,
    XPTI_SPLASH                =  6,
    XPTI_DIALOG                =  7,
    XPTI_DROPDOWN_MENU         =  8,
    XPTI_POPUP_MENU            =  9,
    XPTI_TOOLTIP               = 10,
    XPTI_NOTIFICATION1         = 11,
    XPTI_COMBO                 = 12,
    XPTI_DND                   = 13,
    XPTI_NORMAL                = 14

} xpti; // ≔ x_property_type_index

#define XPTI_QUERY XPTI_WM_WINDOW_TYPE

#define XPTI2XPTV(X) (1 << X)
#define XPTV_TYPE_NOT_FOUND 0
typedef enum {
    XPTV_DESKTOP            = XPTI2XPTV(XPTI_DESKTOP),
    XPTV_DOCK               = XPTI2XPTV(XPTI_DOCK),
    XPTV_TOOLBAR            = XPTI2XPTV(XPTI_TOOLBAR),
    XPTV_MENU               = XPTI2XPTV(XPTI_MENU),
    XPTV_UTILITY            = XPTI2XPTV(XPTI_UTILITY),
    XPTV_SPLASH             = XPTI2XPTV(XPTI_SPLASH),
    XPTV_DIALOG             = XPTI2XPTV(XPTI_DIALOG),
    XPTV_DROPDOWN_MENU      = XPTI2XPTV(XPTI_DROPDOWN_MENU),
    XPTV_POPUP_MENU         = XPTI2XPTV(XPTI_POPUP_MENU),
    XPTV_TOOLTIP            = XPTI2XPTV(XPTI_TOOLTIP),
    XPTV_NOTIFICATION1      = XPTI2XPTV(XPTI_NOTIFICATION1),
    XPTV_COMBO              = XPTI2XPTV(XPTI_COMBO),
    XPTV_DND                = XPTI2XPTV(XPTI_DND),
    XPTV_NORMAL             = XPTI2XPTV(XPTI_NORMAL)

} xptv; // ≔ x_property_type_index
// #define XPTV2XPTI(X) ( log(X)/log(2) )
// There's no such thing as an integer log.
// The above takes -lm and #include <math.h> and is for doubles …
// … though, int result = log(X)/log(2) would otherwise do the job.
// see also: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious

typedef enum {
    XPSI_WM_STATE             = 15,

    XPSI_MODAL                = 16,
    XPSI_STICKY               = 17,
    XPSI_MAXIMIZED_VERT       = 18,
    XPSI_MAXIMIZED_HORZ       = 19,
    XPSI_SHADED               = 20,
    XPSI_SKIP_TASKBAR         = 21,
    XPSI_SKIP_PAGER           = 22,
    XPSI_HIDDEN               = 23,
    XPSI_FULLSCREEN           = 24,
    XPSI_ABOVE                = 25,
    XPSI_BELOW                = 26,
    XPSI_DEMANDS_ATTENTION    = 27,

    XPSI_END                  = 28

} xpsi; // ≔ x_property_state_index

#define XPSI_QUERY XPSI_WM_STATE

#define XPSI2XPSV(X) (1 << (X-15))
#define XPSV_STATE_NOT_FOUND 0
typedef enum {
    XPSV_MODAL                = XPSI2XPSV(XPSI_MODAL),
    XPSV_STICKY               = XPSI2XPSV(XPSI_STICKY),
    XPSV_MAXIMIZED_VERT       = XPSI2XPSV(XPSI_MAXIMIZED_VERT),
    XPSV_MAXIMIZED_HORZ       = XPSI2XPSV(XPSI_MAXIMIZED_HORZ),
    XPSV_SHADED               = XPSI2XPSV(XPSI_SHADED),
    XPSV_SKIP_TASKBAR         = XPSI2XPSV(XPSI_SKIP_TASKBAR),
    XPSV_SKIP_PAGER           = XPSI2XPSV(XPSI_SKIP_PAGER),
    XPSV_HIDDEN               = XPSI2XPSV(XPSI_HIDDEN),
    XPSV_FULLSCREEN           = XPSI2XPSV(XPSI_FULLSCREEN),
    XPSV_ABOVE                = XPSI2XPSV(XPSI_ABOVE),
    XPSV_BELOW                = XPSI2XPSV(XPSI_BELOW),
    XPSV_DEMANDS_ATTENTION    = XPSI2XPSV(XPSI_DEMANDS_ATTENTION)

} xpsv;

void window_query_setup(Display *display);
xptv get_wm_type(Display *display, Window window);
xpsv get_wm_state(Display *display, Window window);

xptv get_wm_type_recursively(Display *display, Window window);
xpsv get_wm_state_recursively(Display *display, Window window);

// NOTE[utility]: some popups in firefox are utility windows (e.g. pushbullet),
// which are apparently like tooltips they seem to be kindof modal and kindof
// tooltipy — not sure what else is utility but we probably don't want to
// rescue focus from utility windows because they kill-self when they lose
// focus.

// we normally don't rescue focus from TYPE_ISA_SPECIAL windows
#define TYPE_ISA_SPECIAL(Y) ( \
        Y & XPTV_DOCK \
     || Y & XPTV_TOOLBAR \
     || Y & XPTV_MENU \
     || Y & XPTV_DIALOG \
     || Y & XPTV_DROPDOWN_MENU \
     || Y & XPTV_POPUP_MENU \
     || Y & XPTV_TOOLTIP \
     || Y & XPTV_NOTIFICATION1 \
     || Y & XPTV_DESKTOP \
     || Y & XPTV_UTILITY \
     )

// NOTE[taskbar]: some popups in eclipse are taskbar-skip windows (e.g.  the
// context help windows.  If they lose focus, they kill-self, so you can't
// click in them if we rescue focus.  I have mixed feelings about allowing
// these to steal focus because I suspect this will allow some windows to steal
// focus that should not be able to steal focus.  Perhaps we should instead
// check harder for parent window groupings or something? XXX

// we also normally don't rescue focus from STATE_ISA SPECIAL windows
#define STATE_ISA_SPECIAL(Y) ( Y & XPSV_MODAL || Y & XPSV_SKIP_TASKBAR )

XID get_wm_group(Display *display, Window window);

bool is_this_atom( int xpti_or_xpsi, Atom atom );

#endif
