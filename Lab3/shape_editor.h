#pragma once

#include <windows.h>

class Shape;

class ShapeObjectsEditor
{
private:
    static const int MAX_SHAPES = 104;

    Shape* pcshape[MAX_SHAPES];
    int shapeCount;

    class ShapeEditor* currentEditor;

public:
    ShapeObjectsEditor();
    ~ShapeObjectsEditor();

    ShapeObjectsEditor(const ShapeObjectsEditor&) = delete;
    ShapeObjectsEditor& operator=(const ShapeObjectsEditor&) = delete;

    void StartPointEditor(HWND hwnd);
    void StartLineEditor(HWND hwnd);
    void StartRectEditor(HWND hwnd);
    void StartEllipseEditor(HWND hwnd);

    void OnLBdown(HWND hwnd);
    void OnLBup(HWND hwnd);
    void OnMouseMove(HWND hwnd);
    void OnPaint(HWND hwnd);
};