#include <stdio.h>

typedef enum { Get, Set, Inc, Dec } op_t;

int _xbacklight_get ();
int _xbacklight_set (op_t type, int value, int steps, int time);
