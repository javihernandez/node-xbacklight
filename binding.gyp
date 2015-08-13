{
  "targets": [
    {
      "target_name": "xbacklight",
      "sources": ["src/xbacklight.cc", "src/xcb.cc"],
      "libraries": ["<!@(pkg-config --libs xcb-randr xcb-atom xcb-aux xcb)"],
      "cflags": ["<!@(pkg-config --cflags xcb-randr xcb-atom xcb-aux xcb)"],
      "ldflags": ["<!@(pkg-config --libs xcb-randr xcb-atom xcb-aux xcb)"]
    }
  ]
}
