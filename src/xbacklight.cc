/*
 * Copyright © 2007 Keith Packard
 * Copyright © 2015 Emergya
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>

#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <node.h>
#include <v8.h>

using namespace v8;

typedef enum { Get, Set, Inc, Dec } op_t;

static char *program_name;

static xcb_atom_t backlight, backlight_new, backlight_legacy;

static void
usage (int exitcode)
{
    fprintf(stderr, "usage: %s [options]\n%s", program_name,
            "  where options are:\n"
            "  -display <display> or -d <display>\n"
            "  -help\n"
            "  -version\n"
            "  -set <percentage> or = <percentage>\n"
            "  -inc <percentage> or + <percentage>\n"
            "  -dec <percentage> or - <percentage>\n"
            "  -get\n"
            "  -time <fade time in milliseconds>\n"
            "  -steps <number of steps in fade>\n");
    exit (exitcode);
}

static void
missing_arg (const char *option)
{
    fprintf(stderr, "%s: %s requires an argument\n", program_name, option);
    usage(1);
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
_xbacklight (int argc, char **argv)
{
    char    *dpy_name = NULL;
    op_t    op = Get;
    int	    value = 0;
    int	    i;
    int	    total_time = 200;	/* ms */
    int	    steps = 20;

    xcb_connection_t *conn;
    xcb_generic_error_t *error;

    xcb_randr_query_version_cookie_t ver_cookie;
    xcb_randr_query_version_reply_t *ver_reply;

    xcb_intern_atom_cookie_t backlight_cookie[2];
    xcb_intern_atom_reply_t *backlight_reply;

    xcb_screen_iterator_t iter;

    program_name = argv[0];

    for (i = 1; i < argc; i++)
    {
	if (!strcmp (argv[i], "-display") || !strcmp ("-d", argv[i]))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    dpy_name = argv[i];
	    continue;
	}
	if (!strcmp (argv[i], "-set") || !strcmp (argv[i], "="))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    op = Set;
	    value = atoi (argv[i]);
	    continue;
	}
	if (argv[i][0] == '=' && isdigit (argv[i][1]))
	{
	    op = Set;
	    value = atoi (argv[i] + 1);
	    continue;
	}
	if (!strcmp (argv[i], "-inc") || !strcmp (argv[i], "+"))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    op = Inc;
	    value = atoi (argv[i]);
	    continue;
	}
	if (argv[i][0] == '+' && isdigit (argv[i][1]))
	{
	    op = Inc;
	    value = atoi (argv[i] + 1);
	    continue;
	}
	if (!strcmp (argv[i], "-dec") || !strcmp (argv[i], "-"))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    op = Dec;
	    value = atoi (argv[i]);
	    continue;
	}
	if (argv[i][0] == '-' && isdigit (argv[i][1]))
	{
	    op = Dec;
	    value = atoi (argv[i] + 1);
	    continue;
	}
	if (!strcmp (argv[i], "-get") || !strcmp (argv[i], "-g"))
	{
	    op = Get;
	    continue;
	}
	if (!strcmp (argv[i], "-time"))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    total_time = atoi (argv[i]);
	    continue;
	}
	if (!strcmp (argv[i], "-steps"))
	{
	    if (++i >= argc) missing_arg (argv[i-1]);
	    steps = atoi (argv[i]);
	    continue;
	}
	if (!strcmp (argv[i], "-help") || !strcmp (argv[i], "-?"))
	{
	    usage (0);
	}
	if (!strcmp (argv[i], "-version"))
	{
	    puts ("0.1.0");
	    exit (0);
	}
	fprintf(stderr, "%s: unrecognized argument '%s'\n",
		program_name, argv[i]);
	usage (1);
    }
    conn = xcb_connect (dpy_name, NULL);
    ver_cookie = xcb_randr_query_version (conn, 1, 2);
    ver_reply = xcb_randr_query_version_reply (conn, ver_cookie, &error);
    if (error != NULL || ver_reply == NULL) {
	int ec = error ? error->error_code : -1;
	fprintf (stderr, "RANDR Query Version returned error %d\n", ec);
	exit (1);
    }
    if (ver_reply->major_version != 1 ||
	ver_reply->minor_version < 2) {
	fprintf (stderr, "RandR version %d.%d too old\n",
		 ver_reply->major_version, ver_reply->minor_version);
	exit (1);
    }
    free (ver_reply);

    backlight_cookie[0] = xcb_intern_atom (conn, 1, strlen("Backlight"), "Backlight");
    backlight_cookie[1] = xcb_intern_atom (conn, 1, strlen("BACKLIGHT"), "BACKLIGHT");

    backlight_reply = xcb_intern_atom_reply (conn, backlight_cookie[0], &error);
    if (error != NULL || backlight_reply == NULL) {
	int ec = error ? error->error_code : -1;
	fprintf (stderr, "Intern Atom returned error %d\n", ec);
	exit (1);
    }

    backlight_new = backlight_reply->atom;
    free (backlight_reply);

    backlight_reply = xcb_intern_atom_reply (conn, backlight_cookie[1], &error);
    if (error != NULL || backlight_reply == NULL) {
	int ec = error ? error->error_code : -1;
	fprintf (stderr, "Intern Atom returned error %d\n", ec);
	exit (1);
    }

    backlight_legacy = backlight_reply->atom;
    free (backlight_reply);

    if (backlight_new == XCB_NONE && backlight_legacy == XCB_NONE) {
	fprintf (stderr, "No outputs have backlight property\n");
	exit (1);
    }

    iter = xcb_setup_roots_iterator (xcb_get_setup (conn));
    while (iter.rem) {
	xcb_screen_t *screen = iter.data;
	xcb_window_t root = screen->root;
	xcb_randr_output_t *outputs;

	xcb_randr_get_screen_resources_cookie_t resources_cookie;
	xcb_randr_get_screen_resources_reply_t *resources_reply;

	resources_cookie = xcb_randr_get_screen_resources (conn, root);
	resources_reply = xcb_randr_get_screen_resources_reply (conn, resources_cookie, &error);
	if (error != NULL || resources_reply == NULL) {
	    int ec = error ? error->error_code : -1;
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

	    cur = backlight_get (conn, output);
	    if (cur != -1)
	    {
		xcb_randr_query_output_property_cookie_t prop_cookie;
		xcb_randr_query_output_property_reply_t *prop_reply;

		prop_cookie = xcb_randr_query_output_property (conn, output, backlight);
		prop_reply = xcb_randr_query_output_property_reply (conn, prop_cookie, &error);

		if (error != NULL || prop_reply == NULL) continue;

		if (prop_reply->range &&
		    xcb_randr_query_output_property_valid_values_length (prop_reply) == 2) {
		    int32_t *values = xcb_randr_query_output_property_valid_values (prop_reply);
		    min = values[0];
		    max = values[1];

		    if (op == Get) {
			//printf ("%f\n", (cur - min) * 100 / (max - min));
                        double return_value;
                        return_value = (cur - min) * 100 / (max - min);
                        return return_value;
		    } else {
			set = value * (max - min) / 100;
			switch (op) {
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
			    xcb_aux_sync (conn);
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
			    backlight_set (conn, output, (long) cur);
			    xcb_flush (conn);
			    usleep (total_time * 1000 / steps);
			}
		    }
		}
		free (prop_reply);
	    }
	}

	free (resources_reply);
	xcb_screen_next (&iter);
    }
    xcb_aux_sync (conn);

    return 0;
}

int int_count ( int num) {
   int count =0;
   if (num == 0) count++;
   while (num !=0) {
       count++;
       num/=10;
   }
   return count;
}

int double_count ( double num) {
   int count =0;
   if (num == 0) count++;
   while (num !=0) {
       count++;
       num/=10;
   }
   return count;
}

/**
 * get:
 *
 * Takes no arguments
 * Returns: The value (0-100) representing the value of the current backlight
 */
Handle<Value>
get (const Arguments& args)
{
    HandleScope scope;
    double curr;
    const char *op[] = {"_xbacklight"};
    curr = _xbacklight(1, (char **) op);
    return scope.Close(Number::New(curr));
};

/**
 * set:
 *
 * Should take as arguments:
 *   value: New value 0-100
 *   steps: Number of steps in fade (Not implemented)
 *   time:  Fade time in milliseconds (Not implemented)
 * Returns: true
 */
Handle<Value>
set (const Arguments& args)
{
    HandleScope scope;
    double value = args[0]->ToNumber()->Value();
    int steps = args[1]->ToNumber()->Value();
    double time = args[2]->ToNumber()->Value();

    // TODO: Remove hard coded values for steps and time
    // Right now we want to get rid of steps, so we're harcoding it, but the
    // module will allow us to use the steps and time when setting a new value.
    // In any case, all this logic will change when we will adding support for
    // the other command-line utilities to the addon.
    //
    steps = 1;
    time = 1;

    char value_s[double_count(value) + 1];
    char steps_s[int_count(steps) + 1];
    char time_s[double_count(time) + 1];

    sprintf(value_s, "%f", value);
    sprintf(steps_s, "%d", steps);
    sprintf(time_s, "%f", time);

    const char *op[] = {"_xbacklight", "-set", value_s,
                        "-steps", steps_s, "-time", time_s};

    _xbacklight(7, (char **) op);

    return scope.Close(Boolean::New("True"));
};

void init(Handle<Object> target) {
    target->Set(String::NewSymbol("get"),
                FunctionTemplate::New(get)->GetFunction());
    target->Set(String::NewSymbol("set"),
                FunctionTemplate::New(set)->GetFunction());
}

NODE_MODULE(xbacklight, init)
