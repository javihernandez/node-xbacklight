/*
 * Copyright 2007 Keith Packard
 * Copyright 2015 Emergya
 *
 * The research leading to these results has received funding from the European
 * Union's Seventh Framework Programme (FP7/2007-2013) under grant agreement
 * n° 289016. More about Cloud4all: http://cloud4all.info
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <node.h>

#include "xcb.h"

using namespace v8;

static xcb_atom_t backlight, backlight_new, backlight_legacy;

typedef struct xcbData {
    char *dpy_name;

    xcb_connection_t *conn;
    // can be deleted from here
    xcb_generic_error_t *error;

    xcb_randr_query_version_cookie_t ver_cookie;
    // can be deleted from here
    xcb_randr_query_version_reply_t *ver_reply;

    xcb_intern_atom_cookie_t backlight_cookie[2];
    // can be deleted from here
    xcb_intern_atom_reply_t *backlight_reply;

    xcb_screen_iterator_t iter;
} xcbData;

xcbData
_get_xcb_data () {
    xcbData data;

    data.dpy_name = NULL;

    data.conn = xcb_connect (data.dpy_name, NULL);
    data.ver_cookie = xcb_randr_query_version (data.conn, 1, 2);
    data.ver_reply = xcb_randr_query_version_reply (data.conn, data.ver_cookie, &data.error);
    if (data.error != NULL || data.ver_reply == NULL) {
        std::ostringstream err_msg;
        int ec = data.error ? data.error->error_code : -1;

        err_msg << "RANDR Query Version returned error " << ec;
        std::string err_str = err_msg.str();
        ThrowException(Exception::Error(
            String::New(err_str.c_str())));
    }
    if (data.ver_reply->major_version != 1 ||
	data.ver_reply->minor_version < 2) {

        std::ostringstream err_msg;

        err_msg << "RANDR version " << data.ver_reply->major_version << "." << data.ver_reply->minor_version << " too old.";
        std::string err_str = err_msg.str();
        ThrowException(Exception::Error(
            String::New(err_str.c_str())));
    }
    free (data.ver_reply);

    data.backlight_cookie[0] = xcb_intern_atom (data.conn, 1, strlen("Backlight"), "Backlight");
    data.backlight_cookie[1] = xcb_intern_atom (data.conn, 1, strlen("BACKLIGHT"), "BACKLIGHT");

    data.backlight_reply = xcb_intern_atom_reply (data.conn, data.backlight_cookie[0], &data.error);
    if (data.error != NULL || data.backlight_reply == NULL) {
        std::ostringstream err_msg;

	int ec = data.error ? data.error->error_code : -1;
        err_msg << "Intern Atom returned error " << ec;
        std::string err_str = err_msg.str();
        ThrowException(Exception::Error(
            String::New(err_str.c_str())));
    }

    backlight_new = data.backlight_reply->atom;
    free (data.backlight_reply);

    data.backlight_reply = xcb_intern_atom_reply (data.conn, data.backlight_cookie[1], &data.error);
    if (data.error != NULL || data.backlight_reply == NULL) {
        std::ostringstream err_msg;

	int ec = data.error ? data.error->error_code : -1;
        err_msg << "Intern Atom returned error " << ec;
        std::string err_str = err_msg.str();
        ThrowException(Exception::Error(
            String::New(err_str.c_str())));
    }

    backlight_legacy = data.backlight_reply->atom;
    free (data.backlight_reply);

    if (backlight_new == XCB_NONE && backlight_legacy == XCB_NONE) {
        std::string err_str ("No outputs have backlight property");
        ThrowException(Exception::Error(
            String::New(err_str.c_str())));
    }

    data.iter = xcb_setup_roots_iterator (xcb_get_setup (data.conn));

    return data;
}

static long
backlight_get (xcb_connection_t *conn, xcb_randr_output_t output)
{
    xcb_generic_error_t *error;
    xcb_randr_get_output_property_reply_t *prop_reply = NULL;
    xcb_randr_get_output_property_cookie_t prop_cookie;
    long value;

    backlight = backlight_new;
    if (backlight != XCB_ATOM_NONE) {
        prop_cookie = xcb_randr_get_output_property (conn, output,
                                                     backlight, XCB_ATOM_NONE,
                                                     0, 4, 0, 0);
        prop_reply = xcb_randr_get_output_property_reply (conn, prop_cookie, &error);
        if (error != NULL || prop_reply == NULL) {
            backlight = backlight_legacy;
            if (backlight != XCB_ATOM_NONE) {
                prop_cookie = xcb_randr_get_output_property (conn, output,
                                                             backlight, XCB_ATOM_NONE,
                                                             0, 4, 0, 0);
                prop_reply = xcb_randr_get_output_property_reply (conn, prop_cookie, &error);
                if (error != NULL || prop_reply == NULL) {
                    return -1;
                }
            }
        }
    }

    if (prop_reply == NULL ||
	prop_reply->type != XCB_ATOM_INTEGER ||
	prop_reply->num_items != 1 ||
	prop_reply->format != 32) {
	value = -1;
    } else {
	value = *((int32_t *) xcb_randr_get_output_property_data (prop_reply));
    }

    free (prop_reply);
    return value;
}

static void
backlight_set (xcb_connection_t *conn, xcb_randr_output_t output, long value)
{
    xcb_randr_change_output_property (conn, output, backlight, XCB_ATOM_INTEGER,
				      32, XCB_PROP_MODE_REPLACE,
				      1, (unsigned char *)&value);
}

int
_xbacklight_set (op_t type, int value, int steps, int time) {
    xcbData data;
    int i;
    int	total_time = 200; /* ms */

    data = _get_xcb_data();

    while (data.iter.rem) {
        xcb_screen_t *screen = data.iter.data;
        xcb_window_t root = screen->root;
        xcb_randr_output_t *outputs;

        xcb_randr_get_screen_resources_cookie_t resources_cookie;
        xcb_randr_get_screen_resources_reply_t *resources_reply;

        resources_cookie = xcb_randr_get_screen_resources (data.conn, root);
        resources_reply = xcb_randr_get_screen_resources_reply (data.conn, resources_cookie, &data.error);
        if (data.error != NULL || resources_reply == NULL) {
            int ec = data.error ? data.error->error_code : -1;
            fprintf (stderr, "RANDR Get Screen Resources returned error %d\n", ec);
            continue;
        }

        outputs = xcb_randr_get_screen_resources_outputs (resources_reply);
        for (int o = 0; o < resources_reply->num_outputs; o++)
        {
            xcb_randr_output_t output = outputs[o];
            double    	cur, new_value, step;
            double	min, max;
            double	set;

            cur = backlight_get (data.conn, output);
            if (cur != -1)
            {
                xcb_randr_query_output_property_cookie_t prop_cookie;
                xcb_randr_query_output_property_reply_t *prop_reply;

                prop_cookie = xcb_randr_query_output_property (data.conn, output, backlight);
                prop_reply = xcb_randr_query_output_property_reply (data.conn, prop_cookie, &data.error);

                if (data.error != NULL || prop_reply == NULL) continue;

                if (prop_reply->range &&
                    xcb_randr_query_output_property_valid_values_length (prop_reply) == 2) {
                    int32_t *values = xcb_randr_query_output_property_valid_values (prop_reply);
                    min = values[0];
                    max = values[1];

                    set = value * (max - min) / 100;
                    switch (type) {
                    case Set:
                        new_value = min + set;
                        break;
                    case Inc:
                        new_value = cur + set;
                        break;
                    case Dec:
                        new_value = cur - set;
                        break;
                    default:
                        xcb_aux_sync (data.conn);
                        return 1;
                    }
                    if (new_value > max) new_value = max;
                    if (new_value < min) new_value = min;
                    step = (new_value - cur) / steps;
                    for (i = 0; i < steps && step != 0; i++)
                    {
                        if (i == steps - 1)
                        cur = new_value;
                        else
                        cur += step;
                        backlight_set (data.conn, output, (long) cur);
                        xcb_flush (data.conn);
                        usleep (total_time * 1000 / steps);
                    }
                }
                free (prop_reply);
            }
        }

        free (resources_reply);
        xcb_screen_next (&data.iter);
    }

    return 1;
}

int
_xbacklight_get () {
    xcbData data;

    data = _get_xcb_data();

    while (data.iter.rem) {
        xcb_screen_t *screen = data.iter.data;
        xcb_window_t root = screen->root;
        xcb_randr_output_t *outputs;

        xcb_randr_get_screen_resources_cookie_t resources_cookie;
        xcb_randr_get_screen_resources_reply_t *resources_reply;

        resources_cookie = xcb_randr_get_screen_resources (data.conn, root);
        resources_reply = xcb_randr_get_screen_resources_reply (data.conn, resources_cookie, &data.error);
        if (data.error != NULL || resources_reply == NULL) {
            int ec = data.error ? data.error->error_code : -1;
            fprintf (stderr, "RANDR Get Screen Resources returned error %d\n", ec);
            continue;
        }

        outputs = xcb_randr_get_screen_resources_outputs (resources_reply);
        for (int o = 0; o < resources_reply->num_outputs; o++)
        {
            xcb_randr_output_t output = outputs[o];
            double      cur;
            double      min, max;

            cur = backlight_get (data.conn, output);

            if (cur != -1)
            {
                xcb_randr_query_output_property_cookie_t prop_cookie;
                xcb_randr_query_output_property_reply_t *prop_reply;

                prop_cookie = xcb_randr_query_output_property (data.conn, output, backlight);
                prop_reply = xcb_randr_query_output_property_reply (data.conn, prop_cookie, &data.error);

                if (data.error != NULL || prop_reply == NULL) continue;

                if (prop_reply->range &&
                    xcb_randr_query_output_property_valid_values_length (prop_reply) == 2) {
                    int32_t *values = xcb_randr_query_output_property_valid_values (prop_reply);
                    min = values[0];
                    max = values[1];

                    int return_value;
                    return_value = (cur - min) * 100 / (max - min);
                    return return_value;
                }
            }
        }
    }

    // Shall we throw an error?
    return 0;
}
