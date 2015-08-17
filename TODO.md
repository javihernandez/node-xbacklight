# TODO

* Add _display_ option
* Re-consider parameters passing to _set_, _inc_ and _dec_. Probably it's best to do something like the following:

```
xbacklight.set(25, {
  steps: 25,
  time: 250,
  display: ":0"
});
```

* Consider the option to add async versions of the public functions

Feel free to take any of these tasks if you want to contribute with __node-xbacklight__.


