#pragma once

#include <windows.h>

class Shape;

class MyEditor
{
private:
    static const int MAX_SHAPES = 104;

    Shape* pshape[MAX_SHAPES];
    int shapeCount;

    Shape* pcurrent;

    long startX, startY;
    long lastX, lastY;
    bool drawing;

    void Draw(HDC hdc, const RECT& clientRect);

public:
    MyEditor();
    ~MyEditor();

    MyEditor(const MyEditor&) = delete;
    MyEditor& operator=(const MyEditor&) = delete;

    void Start(Shape* shape);
    void OnLBdown(HWND hwnd);
    void OnLBup(HWND hwnd);
    void OnMouseMove(HWND hwnd);
    void OnPaint(HWND hwnd);
};