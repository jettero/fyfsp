#include "cache.h"
#include "xquery.h"
#include "window.h"

#include <assert.h>

#ifdef HASH_EMIT_KEYS
#include <unistd.h>
#endif

#ifndef DEBUG_CACHE
#define printf(...)
#endif

bool CACHE_DISABLED = 0;

typedef struct {
    Window window;
    char value_code;
} window_long_lookup_ct;

typedef struct {
    Window window;
    char value_code;
    unsigned long value;

    UT_hash_handle hh;

} window_long_ct;
window_long_ct *window_long_cache = NULL;

void cache_window_long_value(Display *display, Window window, unsigned long value, char value_code) {
    if( CACHE_DISABLED )
        return;

    window_long_ct *item = (window_long_ct *) malloc(sizeof(window_long_ct));
    item->window = window;
    item->value = value;
    item->value_code = value_code;

    assert( value != NO_RESULT );

    // NOTE: When we store a value (or look one up), we compare the value code
    // along with the window id (hence size+1); but when we clear a value, we
    // clear all value codes for the window, so only the window id matters and
    // we use the window id size without the +1 in that lookup.

    HASH_ADD(hh,               // cache-data field
            window_long_cache, // the cache pointer
            window,            // cache key field name
            sizeof(Window)+1,  // cache key size
            item               // the item pointer
            );

    printf("cache-store(0x%lx, %c→%s, %lu/%lx)\n",
            window, value_code, CODE2STR(value_code), item->value, item->value);
    XSelectInput(display, window, PropertyChangeMask);
}

unsigned long cache_window_long_look(Window window, char value_code) {
    if( CACHE_DISABLED )
        return NO_RESULT;

    window_long_ct *item;
    window_long_lookup_ct lk;
    lk.window = window;
    lk.value_code = value_code;

    HASH_FIND(hh, window_long_cache, &lk, sizeof(Window)+1, item);

    if( item ) {
        printf("cache-lookup(0x%lx, %c→%s) → [hit] %lu/%lx\n",
                window, value_code, CODE2STR(value_code), item->value, item->value);
        return item->value;
    }

    printf("cache-lookup(0x%lx, %c→%s) → [miss]\n",
            window, value_code, CODE2STR(value_code));

    return NO_RESULT;
}

bool _cache_kill_window_callback(Window window) {
    window_long_ct *item, *tmp;

    HASH_ITER(hh, window_long_cache, item, tmp) {
        if (item->window == window) {
            printf("  kill(0x%lx, %c→%s, %lu/%lx)\n",
                    item->window, item->value_code, CODE2STR(item->value_code), item->value, item->value);
            HASH_DEL(window_long_cache, item);
            free(item);
        }
    }

    return RECURSE_CONTINUE;
}

void cache_kill_window(Display *display, Window window) {
    CACHE_DISABLED = 1;

    Window top_window = find_top_window(display, window);

    printf("cache-kill(0x%lx→0x%lx)\n", window, top_window);
    recurse_window_tree(display, top_window, _cache_kill_window_callback);

    CACHE_DISABLED = 0;
}

bool _cache_kill_window_state_related_callback(Window window) {
    window_long_ct *item, *tmp;

    HASH_ITER(hh, window_long_cache, item, tmp) {
        if (item->window == window) {
            switch(item->value_code) {
                case CACHE_WM_STATE_RECURSIVELY:
                    printf("  kill-sr(0x%lx, %c→%s, %lu/%lx)\n",
                            item->window, item->value_code,
                            CODE2STR(item->value_code),
                            item->value, item->value);
                    HASH_DEL(window_long_cache, item);
                    free(item);
            }
        }
    }

    return RECURSE_CONTINUE;
}

void cache_kill_window_state_related(Display *display, Window window) {
    CACHE_DISABLED = 1;

    Window top_window = find_top_window(display, window);

    printf("cache-kill-state-related(0x%lx→0x%lx)\n", window, top_window);
    recurse_window_tree(display, top_window, _cache_kill_window_state_related_callback);

    CACHE_DISABLED = 0;
}
