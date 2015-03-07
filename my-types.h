#ifndef MY_TYPES_H
#define MY_TYPES_H

#include <my-types.h>

#define ATOMS 15
char *atom_str[] = { "_NET_WM_WINDOW_TYPE", "_NET_WM_WINDOW_TYPE_DESKTOP", "_NET_WM_WINDOW_TYPE_DOCK",
        "_NET_WM_WINDOW_TYPE_TOOLBAR", "_NET_WM_WINDOW_TYPE_MENU", "_NET_WM_WINDOW_TYPE_UTILITY",
        "_NET_WM_WINDOW_TYPE_SPLASH", "_NET_WM_WINDOW_TYPE_DIALOG", "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
        "_NET_WM_WINDOW_TYPE_POPUP_MENU", "_NET_WM_WINDOW_TYPE_TOOLTIP", "_NET_WM_WINDOW_TYPE_NOTIFICATION",
        "_NET_WM_WINDOW_TYPE_COMBO", "_NET_WM_WINDOW_TYPE_DND", "_NET_WM_WINDOW_TYPE_NORMAL" };

Atom atom[ATOMS];
#define QUERY_WM_WINDOW_TYPE 0
#define WM_DESKTOP           1
#define WM_DOCK              2
#define WM_TOOLBAR           3
#define WM_MENU              4
#define WM_UTILITY           5
#define WM_SPLASH            6
#define WM_DIALOG            7
#define WM_DROPDOWN_MENU     8
#define WM_POPUP_MENU        9
#define WM_TOOLTIP          10
#define WM_NOTIFICATION     11
#define WM_COMBO            12
#define WM_DND              13
#define WM_NORMAL           14

#endif

