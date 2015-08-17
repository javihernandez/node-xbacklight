# node-xbacklight
Node.js port of [Xbacklight](http://cgit.freedesktop.org/xorg/app/xbacklight), the command line utility for GNU/Linux.

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

### Installation

#### By adding it as a dependency of your project

Until it is published in npm, add it like this in the _dependencies_ section of your _package.json_ file:

```
"dependencies" : {
       "xbacklight": "git@github.com:GPII/node-xbacklight.git"
   }
```

Load it as a regular node module:

```var xbacklight = require("xbacklight");```

#### By cloning this repository

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
  * _value_ - Value range is 0-100
  * _steps_ - Number of steps
  * _time_  - Time in ms
* __inc__ - Increase the value
  * _value_ - Value range is 0-100
  * _steps_ - Number of steps
  * _time_  - Time in ms
* __dec__ - Decrease the value
  * _value_ - Value range is 0-100
  * _steps_ - Number of steps
  * _time_  - Time in ms


### Usage

```
// Load xbacklight
var xbacklight = require("xbacklight");

// Get the current value
xbacklight.get();

// Set brightness to 25%
xbacklight.set(25);

// Increase in 25% the current value and in 2 steps
xbacklight.inc(25, 2);

// Decrease in 50% the current value and take 500 ms to fade out
xbacklight.dec(50, null, 500);

```

Note that since both _steps_ and _time_ are optional arguments, if you only want to specify the _time_, you need to pass _null_ as the value for _steps_.

## Contributing

Any question? Did you found a bug? File an [issue](https://github.com/GPII/node-xbacklight/issues).

Do you want to contribute with source code?
1.  Fork the repository on Github
2.  Create a named feature branch (like `add_component_x`)
3.  Write your change
4.  Submit a pull request using Github

