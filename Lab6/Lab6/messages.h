#pragma once

#include <windows.h>

#define WM_OBJECT_READY (WM_APP + 1)
#define WM_OBJECT_DONE  (WM_APP + 2)
#define WM_DO_GRAPH     (WM_APP + 3)   // Lab6 -> Obj3


#define DATA_PARAMS 1

// Lab6 -> Object2
struct Params
{
    int n;
    double minValue;
    double maxValue;
};