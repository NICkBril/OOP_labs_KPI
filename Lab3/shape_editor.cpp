#include "shape_editor.h"
#include "shape.h"
#include "editor.h"

#include <algorithm>

namespace {
    const int MAX_SHAPES = 104;
}

class ShapeEditor : public Editor
{
protected:
    Shape** pcshape;
    int* shapeCount;

    HWND hwnd;

    long startX, startY;
    long lastX, lastY;

    bool drawing;

    virtual Shape* CreateShape(long x1, long y1, long x2, long y2) = 0;

    virtual void DrawOutline(HDC hdc, long x1, long y1, long x2, long y2) = 0;

    void DrawPreview(HDC hdc, long x2, long y2)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(
            hdc, GetStockObject(NULL_BRUSH)
        );

        DrawOutline(hdc, startX, startY, x2, y2);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);

        DeleteObject(pen);
    }

public:
    ShapeEditor(Shape** shapes, int* count, HWND window)
        : pcshape(shapes),
        shapeCount(count),
        hwnd(window),

        startX(0),
        startY(0),
        lastX(0),
        lastY(0),
        drawing(false) {
    }

    void OnLBdown(HWND window) override
    {
        if (drawing) {
            return;
        }

        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        startX = point.x;
        startY = point.y;
        lastX = startX;
        lastY = startY;

        drawing = true;
        SetCapture(window);
    }

    void OnLBup(HWND window) override
    {
        if (!drawing) {
            return;
        }

        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        drawing = false;
        ReleaseCapture();

        if (*shapeCount < MAX_SHAPES) {
            Shape* shape = CreateShape(startX, startY, point.x, point.y);

            if (shape != nullptr) {
                pcshape[*shapeCount] = shape;
                ++(*shapeCount);
            }
        }

        InvalidateRect(window, NULL, TRUE);
    }

    void OnMouseMove(HWND window) override
    {
        if (!drawing) {
            return;
        }

        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        lastX = point.x;
        lastY = point.y;

        InvalidateRect(window, NULL, TRUE);
    }

    void OnPaint(HWND window) override
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);

        for (int i = 0; i < *shapeCount; ++i) {
            if (pcshape[i] != nullptr) {
                pcshape[i]->Show(hdc);
            }
        }

        if (drawing) {
            DrawPreview(hdc, lastX, lastY);
        }

        EndPaint(window, &ps);
    }
};

class PointEditor : public ShapeEditor
{
public:
    using ShapeEditor::ShapeEditor;

protected:
    Shape* CreateShape(long x1, long y1, long, long) override
    {
        Shape* shape = new PointShape();
        shape->Set(x1, y1, x1, y1);

        return shape;
    }

    void DrawOutline(HDC hdc, long x1, long y1, long, long) override
    {
        SetPixel(hdc, x1, y1, RGB(0, 0, 0));
    }
};

class LineEditor : public ShapeEditor
{
public:
    using ShapeEditor::ShapeEditor;

protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        Shape* shape = new LineShape();
        shape->Set(x1, y1, x2, y2);
        return shape;
    }

    void DrawOutline(HDC hdc, long x1, long y1, long x2, long y2) override
    {
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
    }
};

class RectEditor : public ShapeEditor
{
public:
    using ShapeEditor::ShapeEditor;

protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        long left = (std::min)(x1, x2);
        long top = (std::min)(y1, y2);
        long right = (std::max)(x1, x2);
        long bottom = (std::max)(y1, y2);

        Shape* shape = new RectShape();
        shape->Set(left, top, right, bottom);

        return shape;
    }

    void DrawOutline(HDC hdc, long x1, long y1, long x2, long y2) override
    {
        long left = (std::min)(x1, x2);
        long top = (std::min)(y1, y2);
        long right = (std::max)(x1, x2);
        long bottom = (std::max)(y1, y2);

        Rectangle(hdc, left, top, right, bottom);
    }
};

class EllipseEditor : public ShapeEditor
{
public:
    using ShapeEditor::ShapeEditor;

protected:
    Shape* CreateShape(long cx, long cy, long x2, long y2) override
    {
        long radiusX = (x2 >= cx) ? x2 - cx : cx - x2;
        long radiusY = (y2 >= cy) ? y2 - cy : cy - y2;

        long left = cx - radiusX;
        long top = cy - radiusY;
        long right = cx + radiusX;
        long bottom = cy + radiusY;

        Shape* shape = new EllipseShape();
        shape->Set(left, top, right, bottom);

        return shape;
    }

    void DrawOutline(HDC hdc, long cx, long cy, long x2, long y2) override
    {
        long radiusX = (x2 >= cx) ? x2 - cx : cx - x2;
        long radiusY = (y2 >= cy) ? y2 - cy : cy - y2;

        long left = cx - radiusX;
        long top = cy - radiusY;
        long right = cx + radiusX;
        long bottom = cy + radiusY;

        Ellipse(hdc, left, top, right, bottom);
    }
};

ShapeObjectsEditor::ShapeObjectsEditor()
    : shapeCount(0), currentEditor(nullptr) {
    for (int i = 0; i < MAX_SHAPES; ++i) {
        pcshape[i] = nullptr;
    }
}

ShapeObjectsEditor::~ShapeObjectsEditor()
{
    delete currentEditor;

    for (int i = 0; i < shapeCount; ++i) {
        delete pcshape[i];
    }
}

void ShapeObjectsEditor::StartPointEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new PointEditor(pcshape, &shapeCount, hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartLineEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new LineEditor(pcshape, &shapeCount, hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartRectEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new RectEditor(pcshape, &shapeCount, hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartEllipseEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new EllipseEditor(pcshape, &shapeCount, hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::OnLBdown(HWND hwnd)
{
    if (currentEditor) {
        currentEditor->OnLBdown(hwnd);
    }
}

void ShapeObjectsEditor::OnLBup(HWND hwnd)
{
    if (currentEditor) {
        currentEditor->OnLBup(hwnd);
    }
}

void ShapeObjectsEditor::OnMouseMove(HWND hwnd)
{
    if (currentEditor) {
        currentEditor->OnMouseMove(hwnd);
    }
}

void ShapeObjectsEditor::OnPaint(HWND hwnd)
{
    if (currentEditor) {
        currentEditor->OnPaint(hwnd);
    }
    else {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
}