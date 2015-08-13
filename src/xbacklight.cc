/*
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


#include <node.h>
#include <v8.h>

#include "xcb.h"

using namespace v8;

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
    int curr;
    curr = _xbacklight_get();
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

    int value = args[0]->ToNumber()->Value();
    int steps = args[1]->ToNumber()->Value();
    int time = args[2]->ToNumber()->Value();

    // Set defaults when these come empty
    if (!args[1]->IsNumber()) {
        steps = 1;
    }

    if (!args[2]->IsNumber()) {
        time = 1;
    }

    _xbacklight_set(Set, value, steps, time);

    return scope.Close(Boolean::New("True"));
};

void init(Handle<Object> target) {
    target->Set(String::NewSymbol("get"),
                FunctionTemplate::New(get)->GetFunction());
    target->Set(String::NewSymbol("set"),
                FunctionTemplate::New(set)->GetFunction());
}

NODE_MODULE(xbacklight, init)
