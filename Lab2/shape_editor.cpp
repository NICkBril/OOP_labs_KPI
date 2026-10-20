#include "shape_editor.h"
#include "shape.h"
#include "editor.h"

#include <windowsx.h>

const int MAX_SHAPES = 103;

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

    virtual void DrawPreview(HDC hdc, long x, long y)
    {
        if (GetShapeType() == 0) {
            return;
        }

        HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush =
            (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        SetROP2(hdc, R2_NOTXORPEN);

        if (GetShapeType() == 1) {
            MoveToEx(hdc, startX, startY, NULL);
            LineTo(hdc, x, y);
        } else if (GetShapeType() == 2) {
            long left = 2 * startX - x;
            long top = 2 * startY - y;

            long x1 = (left < x) ? left : x;
            long y1 = (top < y) ? top : y;
            long x2 = (left > x) ? left : x;
            long y2 = (top > y) ? top : y;

            Rectangle(hdc, x1, y1, x2, y2);
        } else if (GetShapeType() == 3) {
            Ellipse(hdc, startX, startY, x, y);
        }

        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    virtual int GetShapeType() = 0;

public:
    ShapeEditor(Shape** shapes, int* count, HWND window)
    {
        pcshape = shapes;
        shapeCount = count;
        hwnd = window;

        startX = 0;
        startY = 0;
        lastX = 0;
        lastY = 0;
        drawing = false;
    }

    void OnLBdown(HWND window) override
    {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        startX = point.x;
        startY = point.y;
        lastX = point.x;
        lastY = point.y;

        drawing = true;
        SetCapture(window);
    }

    void OnLBup(HWND window) override
    {
        if (!drawing) {
            return;
        }

        if (GetShapeType() != 0) {
            HDC hdc = GetDC(window);
            DrawPreview(hdc, lastX, lastY);
            ReleaseDC(window, hdc);
        }

        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        long endX = point.x;
        long endY = point.y;

        drawing = false;
        ReleaseCapture();


        if (*shapeCount < MAX_SHAPES) {
            Shape* shape = CreateShape(
                startX, startY, endX, endY
            );

            if (shape != nullptr) {
                shape->Set(startX, startY, endX, endY);
                pcshape[*shapeCount] = shape;
                (*shapeCount)++;
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

        HDC hdc = GetDC(window);

        if (GetShapeType() != 0) {
            DrawPreview(hdc, lastX, lastY);
        }


        lastX = point.x;
        lastY = point.y;

        if (GetShapeType() != 0) {
            DrawPreview(hdc, lastX, lastY);
        }


        ReleaseDC(window, hdc);
    }

    void OnPaint(HWND window) override
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window, &ps);

        for (int i = 0; i < *shapeCount; i++) {
            if (pcshape[i] != nullptr) {
                pcshape[i]->Show(hdc);
            }
        }

        if (drawing && GetShapeType() != 0) {
            DrawPreview(hdc, lastX, lastY);
        }

        EndPaint(window, &ps);
    }
};

class PointEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long, long, long, long) override
    {
        return new PointShape();
    }

    int GetShapeType() override
    {
        return 0;
    }

public:
    using ShapeEditor::ShapeEditor;
};

class LineEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long, long, long, long) override
    {
        return new LineShape();
    }

    int GetShapeType() override
    {
        return 1;
    }

public:
    using ShapeEditor::ShapeEditor;
};

class RectEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        long oppositeX = 2 * x1 - x2;
        long oppositeY = 2 * y1 - y2;

        long left = (oppositeX < x2) ? oppositeX : x2;
        long top = (oppositeY < y2) ? oppositeY : y2;
        long right = (oppositeX > x2) ? oppositeX : x2;
        long bottom = (oppositeY > y2) ? oppositeY : y2;

        Shape* shape = new RectShape(false);
        shape->Set(left, top, right, bottom);

        return shape;
    }

    int GetShapeType() override
    {
        return 2;
    }

public:
    using ShapeEditor::ShapeEditor;
};

class EllipseEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long, long, long, long) override
    {
        return new EllipseShape(true, RGB(0, 255, 255));
    }

    int GetShapeType() override
    {
        return 3;
    }

public:
    using ShapeEditor::ShapeEditor;
};

ShapeObjectsEditor::ShapeObjectsEditor()
{
    pcshape = new Shape * [MAX_SHAPES];

    for (int i = 0; i < MAX_SHAPES; i++) {
        pcshape[i] = nullptr;
    }

    shapeCount = 0;
    currentEditor = nullptr;
}

ShapeObjectsEditor::~ShapeObjectsEditor()
{
    delete currentEditor;

    for (int i = 0; i < shapeCount; i++) {
        delete pcshape[i];
    }

    delete[] pcshape;
}

void ShapeObjectsEditor::StartPointEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new PointEditor(pcshape, &shapeCount, hwnd);
    SetWindowText(hwnd, L"Графічний редактор - Точка");
}

void ShapeObjectsEditor::StartLineEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new LineEditor(pcshape, &shapeCount, hwnd);
    SetWindowText(hwnd, L"Графічний редактор - Лінія");
}

void ShapeObjectsEditor::StartRectEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new RectEditor(pcshape, &shapeCount, hwnd);
    SetWindowText(hwnd, L"Графічний редактор - Прямокутник");
}

void ShapeObjectsEditor::StartEllipseEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new EllipseEditor(pcshape, &shapeCount, hwnd);
    SetWindowText(hwnd, L"Графічний редактор - Еліпс");
}


void ShapeObjectsEditor::OnLBdown(HWND hwnd)
{
    if (currentEditor != nullptr) {
        currentEditor->OnLBdown(hwnd);
    }
}

void ShapeObjectsEditor::OnLBup(HWND hwnd)
{
    if (currentEditor != nullptr) {
        currentEditor->OnLBup(hwnd);
    }
}

void ShapeObjectsEditor::OnMouseMove(HWND hwnd)
{
    if (currentEditor != nullptr) {
        currentEditor->OnMouseMove(hwnd);
    }
}

void ShapeObjectsEditor::OnPaint(HWND hwnd)
{
    if (currentEditor != nullptr) {
        currentEditor->OnPaint(hwnd);
    }
}
