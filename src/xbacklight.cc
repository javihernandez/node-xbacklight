/*
 * Copyright 2015 Emergya
 * Copyright 2015 Raising the Floor
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


#include <nan.h>

#include "xcb.h"

using namespace v8;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Isolate;
using v8::Object;
using v8::String;
using Nan::GetFunction;
using Nan::New;
using Nan::Set;

void _set_wrapper (op_t op, const Nan::FunctionCallbackInfo<v8::Value>& info)
{
    int value = info[0]->ToNumber()->Value();
    int steps = info[1]->ToNumber()->Value();
    int time = info[2]->ToNumber()->Value();

    // Set defaults when these come empty
    if (!info[1]->IsNumber()) {
        steps = 20;
    }
    if (!info[2]->IsNumber()) {
        time = 200;
    }

    _xbacklight_set(op, value, steps, time, info);
    info.GetReturnValue().Set(Boolean::New(info.GetIsolate(), "True"));
}

/**
 * get:
 *
 * Takes no arguments
 * Returns: The current backlight's percentage (0-100)
 */
NAN_METHOD(get)
{
    int curr;
    curr = _xbacklight_get(info);
    info.GetReturnValue().Set(Number::New(info.GetIsolate(), curr));
};

/**
 * set:
 *
 * Should take as arguments:
 *   value: The desired percentage (0-100)
 *   steps: Number of steps in fade
 *   time:  Fade time in milliseconds
 * Returns: true
 */
NAN_METHOD(set)
{
    _set_wrapper(SetOp, info);
};

/**
 * inc:
 *
 * Should take as arguments:
 *   value: Percentage to increase
 *   steps: Number of steps in fade
 *   time:  Fade time in milliseconds
 * Returns: true
 */
NAN_METHOD(inc)
{
    _set_wrapper(IncOp, info);
};

/**
 * dec:
 *
 * Should take as arguments:
 *   value: Percentage to decrease
 *   steps: Number of steps in fade
 *   time:  Fade time in milliseconds
 * Returns: true
 */
NAN_METHOD(dec)
{
    _set_wrapper(DecOp, info);
};

NAN_MODULE_INIT(init) {
    Nan::Set(target, New<String>("get").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(get)).ToLocalChecked());
    Nan::Set(target, New<String>("set").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(set)).ToLocalChecked());
    Nan::Set(target, New<String>("inc").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(inc)).ToLocalChecked());
    Nan::Set(target, New<String>("dec").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(dec)).ToLocalChecked());
}

NODE_MODULE(xbacklight, init)
