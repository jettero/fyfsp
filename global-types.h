#ifndef GLOBAL_TYPES_H
#define GLOBAL_TYPES_H

#include <X11/Xlib.h>

typedef enum {
    false,
    true
} bool;

#include <stdio.h>
#define text_error(...) fprintf(stderr, "%s:%d ", __FILE__, __LINE__); fprintf(stderr, ##__VA_ARGS__);


#ifdef DEBUG_LOG_TO_FILE
char *current_time_string();
#define printf(F,...) printf("%s " F, current_time_string(), ##__VA_ARGS__)
#endif

#ifndef DEBUG
# define printf(...)
# define setlinebuf(...);
#endif

#define Status2STR(X) ( \
    status == BadWindow  ? "bad window"  : \
    status == BadRequest ? "bad request" : \
    status == BadValue   ? "bad value"   : \
    status == BadMatch   ? "bad match"   : \
    status == Success    ? "(success)"   : \
    "<unknown status code>" \
)

#endif
