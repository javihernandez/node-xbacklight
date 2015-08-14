/*
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

var jqUnit = require("jqUnit"),
    xbacklight = require("../build/Release/xbacklight.node");

jqUnit.module("Xbacklight module tests");

jqUnit.test("Checking the availability of exported functions", function () {
    jqUnit.expect(8);

    var methods = ["get", "set", "dec", "inc"];
    for (var method in methods) {
        jqUnit.assertTrue("Checking availability of method '" + method + "'",
                          (methods[method] in xbacklight));
        jqUnit.assertEquals("Type of xbacklight." + method + " is function",
                            "function", typeof(xbacklight[methods[method]])); 
    }
});

jqUnit.test("Checking that the functions return what they should", function () {
    jqUnit.expect(4);

    var backlight = xbacklight.get();
    jqUnit.assertEquals("get returns a number", "number", typeof(backlight));

    jqUnit.assertTrue("set returns true", xbacklight.set(backlight));
    jqUnit.assertTrue("inc returns true", xbacklight.inc(0));
    jqUnit.assertTrue("dec returns true", xbacklight.dec(0));
});

jqUnit.test("Checking that the 'set', 'inc' and 'dec' functions can handle optional parameters", function () {
    jqUnit.expect(12);

    var backlight = xbacklight.get();

    var funcs = [xbacklight.set, xbacklight.inc, xbacklight.dec];
    for (var i=0; i < funcs.length; i++) {
        jqUnit.assertTrue("Combination of parameters #1", funcs[i](1, null, null));
        jqUnit.assertTrue("Combination of parameters #2", funcs[i](1, 1, null));
        jqUnit.assertTrue("Combination of parameters #3", funcs[i](1, null, 1));
        jqUnit.assertTrue("Combination of parameters #4", funcs[i](1, 1, 1));
    };

    xbacklight.set(backlight);
});

jqUnit.test("Checking that 'set', 'inc' and 'get' functions actually change the xbacklight in the system", function () {
    jqUnit.expect(5);

    var backlight = xbacklight.get();

    xbacklight.set(100);
    var max = xbacklight.get();

    xbacklight.set(0);
    var min = xbacklight.get();

    xbacklight.set(25);
    var cur = xbacklight.get();

    jqUnit.assertNotEquals("Ensuring that 'set' works properly - Previous and current values are different", min, cur);
    jqUnit.assertTrue("The value must be bigger than before", (cur > min));

    xbacklight.set(min);
    cur = xbacklight.get();
    jqUnit.assertTrue("The value must be smaller than before", (cur == min));

    xbacklight.inc(25);
    cur = xbacklight.get();
    jqUnit.assertTrue("Ensuring that 'inc' works - The value has increased", (cur > min));
    xbacklight.dec(25);
    cur = xbacklight.get();
    jqUnit.assertTrue("Ensuring that 'dec' works - The value has decreased", (cur == min));

    xbacklight.set(backlight);
});
