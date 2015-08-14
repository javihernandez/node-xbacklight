# node-xbacklight
Node.js port of GNU/Linux' [Xbacklight](http://cgit.freedesktop.org/xorg/app/xbacklight) command line utility.

Xbacklight is used to adjust the backlight brightness where supported.
It uses the RandR extension to find all outputs on the X server
supporting backlight brightness control and changes them all in the
same way.

With Xbacklight you can:
* Get the current backlight brightness.
* Set the backlight brightness to the specified level.
* Increase and decrease the brightness by the specified amount.

Take into account that the brightness value is always represented as a percentage of the maximum brightness supported.

When setting a new value, you can also specify:
* The number of steps to take while fading. Default is 20.
* Length of time to spend fading the backlight between old and new value.  Default is 200 ms.

## Getting started

### By adding it as a dependency of your project

Until it is published in npm, add it like this in the _dependencies_ section of your _package.json_ file:

```
"dependencies" : {
       "xbacklight": "git@github.com:GPII/node-xbacklight.git"
   }
```

Load it as a regular node module:

```var xbacklight = require("xbacklight");```

### By cloning this repository

After cloning it, run

```npm install```

Load the module:

```var xbacklight = require("build/Release/xbacklight.node");```

### Build dependencies

In addition to the regular requirements for creating a NodeJS addon, we depend on:
* __xcb__: XCB - X-protocol C Binding
* __xcb-atom__: XCB Atom library - XCB atom cache
* __xcb-aux__: XCB Aux library - XCB convenient functions
* __xcb-randr >= 1.2__: XCB RandR - XCB RandR Extension

## API

* __get__ - Get the current value of xbacklight
* __set__ - Set the value for xbacklight
  * value - Value range is 0-100
  * steps - Number of steps
  * time  - Time in ms
* __inc__ - Increase the value
  * value - Value range is 0-100
  * steps - Number of steps
  * time  - Time in ms
* __dec__ - Decrease the value
  * value - Value range is 0-100
  * steps - Number of steps
  * time  - Time in ms

## Contributing

Any question? Did you found a bug? File an [issue](https://github.com/GPII/node-xbacklight/issues).

Do you want to contribute with source code?

1.  Fork the repository on Github
2.  Create a named feature branch (like `add_component_x`)
3.  Write your change
4.  Submit a pull request using Github

