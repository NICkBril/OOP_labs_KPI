#pragma once

#include <windows.h>

class Shape;
class ShapeEditor;

class ShapeObjectsEditor
{
private:
    ShapeEditor* currentEditor;

    Shape** pcshape;
    int shapeCount;

public:
    ShapeObjectsEditor();
    ~ShapeObjectsEditor();

    void StartPointEditor(HWND hwnd);
    void StartLineEditor(HWND hwnd);
    void StartRectEditor(HWND hwnd);
    void StartEllipseEditor(HWND hwnd);

    void OnLBdown(HWND hwnd);
    void OnLBup(HWND hwnd);
    void OnMouseMove(HWND hwnd);
    void OnPaint(HWND hwnd);
};