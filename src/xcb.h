#include <stdio.h>

typedef enum { GetOp, SetOp, IncOp, DecOp } op_t;

int _xbacklight_get (const Nan::FunctionCallbackInfo<v8::Value>& info);
int _xbacklight_set (op_t type, int value, int steps, int time, const Nan::FunctionCallbackInfo<v8::Value>& info);
