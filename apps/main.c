/*
 * Twin - A Tiny Window System
 * Copyright (c) 2024 National Cheng Kung University, Taiwan
 * Copyright (c) 2004 Keith Packard <keithp@keithp.com>
 * All rights reserved.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "apps_calc.h"
#include "apps_clock.h"
#include "apps_hello.h"
#include "apps_line.h"
#include "apps_multi.h"
#include "apps_spline.h"

#define WIDTH 640
#define HEIGHT 480

#define ASSET_PATH "assets/"

/**
 * Load the background pixmap from storage.
 * @filename: File name of a PNG background.
 *
 * Return a default pattern if the load of @filename fails.
 */
static twin_pixmap_t *load_background(twin_screen_t *screen,
                                      const char *filename)
{
#if defined(CONFIG_LOADER_PNG)
    twin_pixmap_t *raw_background = twin_png_to_pixmap(filename, TWIN_ARGB32);
    if (!raw_background) /* Fallback to a default pattern */
        return twin_make_pattern();

    if (screen->height == raw_background->height &&
        screen->width == raw_background->width)
        return raw_background;

    /* Scale as needed. */
    twin_pixmap_t *scaled_background =
        twin_pixmap_create(TWIN_ARGB32, screen->width, screen->height);
    if (!scaled_background) {
        twin_pixmap_destroy(raw_background);
        return twin_make_pattern();
    }
    twin_fixed_t sx, sy;
    sx = twin_fixed_div(twin_int_to_fixed(raw_background->width),
                        twin_int_to_fixed(screen->width));
    sy = twin_fixed_div(twin_int_to_fixed(raw_background->height),
                        twin_int_to_fixed(screen->height));

    twin_matrix_scale(&raw_background->transform, sx, sy);
    twin_operand_t srcop = {
        .source_kind = TWIN_PIXMAP,
        .u.pixmap = raw_background,
    };
    twin_composite(scaled_background, 0, 0, &srcop, 0, 0, NULL, 0, 0,
                   TWIN_SOURCE, screen->width, screen->height);

    twin_pixmap_destroy(raw_background);

    return scaled_background;
#else
    return twin_make_pattern();
#endif
}

static twin_context_t *tx = NULL;

static void cleanup(void)
{
    twin_destroy(tx);
    tx = NULL;
}

static void sigint_handler(int sig)
{
    (void) sig;
    cleanup();
    exit(1);
}

int main(void)
{
    tx = twin_create(WIDTH, HEIGHT);
    if (!tx)
        return 1;

    /* Register the callback function to release allocated resources when SIGINT
     * is caught or the program is about to exit.
     * This is important for long-time execution of the main loop, from which
     * the user usually exits with input events.
     */
    atexit(cleanup);
    signal(SIGINT, sigint_handler);

    twin_screen_set_background(
        tx->screen, load_background(tx->screen, ASSET_PATH "/tux.png"));

#if defined(CONFIG_DEMO_MULTI)
    apps_multi_start(tx->screen, "Demo", 100, 100, 400, 400);
#endif
#if defined(CONFIG_DEMO_HELLO)
    apps_hello_start(tx->screen, "Hello, World", 0, 0, 200, 200);
#endif
#if defined(CONFIG_DEMO_CLOCK)
    apps_clock_start(tx->screen, "Clock", 10, 10, 200, 200);
#endif
#if defined(CONFIG_DEMO_CALCULATOR)
    apps_calc_start(tx->screen, "Calculator", 100, 100, 200, 200);
#endif
#if defined(CONFIG_DEMO_LINE)
    apps_line_start(tx->screen, "Line", 0, 0, 200, 200);
#endif
#if defined(CONFIG_DEMO_SPLINE)
    apps_spline_start(tx->screen, "Spline", 20, 20, 400, 400);
#endif

    twin_dispatch();

    return 0;
}
