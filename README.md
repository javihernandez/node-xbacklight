# node-xbacklight
Node.js binding to GNU/Linux xbacklight command line utility, the code was originally copied from http://cgit.freedesktop.org/xorg/app/xbacklight.

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

In addition to the regular requirements for creating a NodeJS addon, pkg-config must be satisfied by having the following:
* xcb: XCB - X-protocol C Binding
* xcb-atom: XCB Atom library - XCB atom cache
* xcb-aux: XCB Aux library - XCB convenient functions
* xcb-randr >= 1.2: XCB RandR - XCB RandR Extension

## API

Right now it only supports simple calls to _get_ and _set_, but it'll be improved to support the rest of options that the original _xbacklight_ command-line utility supports.

* get - Returns the current value of xbacklight
* set - Set the value for xbacklight
  * int value - Value range is 0-100

## Contributing

1.  Fork the repository on Github
2.  Create a named feature branch (like `add_component_x`)
3.  Write your change
4.  Submit a Pull Request using Github

