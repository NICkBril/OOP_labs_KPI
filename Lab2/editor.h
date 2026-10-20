#pragma once

#include <windows.h>

class Editor
{
public:
    virtual void OnLBdown(HWND hwnd) = 0;
    virtual void OnLBup(HWND hwnd) = 0;
    virtual void OnMouseMove(HWND hwnd) = 0;
    virtual void OnPaint(HWND hwnd) = 0;

    virtual ~Editor() {}
};
