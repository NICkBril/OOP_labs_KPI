#include "shape_editor.h"
#include "shape.h"
#include "editor.h"

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

    virtual void DrawOutline(HDC hdc, long x, long y)
    {
    }

    void DrawPreview(HDC hdc, long x, long y)
    {
        HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush =
            (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        int oldRop = SetROP2(hdc, R2_NOTXORPEN);

        DrawOutline(hdc, x, y);

        SetROP2(hdc, oldRop);
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

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
        if (!drawing)
        {
            return;
        }

        POINT point;
        GetCursorPos(&point);
        ScreenToClient(window, &point);

        long endX = point.x;
        long endY = point.y;

        HDC hdc = GetDC(window);
        DrawPreview(hdc, lastX, lastY);
        ReleaseDC(window, hdc);

        drawing = false;
        ReleaseCapture();

        if (*shapeCount < MAX_SHAPES) {
            Shape* shape = CreateShape(
                startX, startY, endX, endY
            );

            if (shape != nullptr) {
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

        DrawPreview(hdc, lastX, lastY);

        lastX = point.x;
        lastY = point.y;

        DrawPreview(hdc, lastX, lastY);

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

        if (drawing) {
            DrawPreview(hdc, lastX, lastY);
        }

        EndPaint(window, &ps);
    }
};

class PointEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        Shape* shape = new PointShape();
        shape->Set(x1, y1, x2, y2);

        return shape;
    }

public:
    using ShapeEditor::ShapeEditor;
};

class LineEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        Shape* shape = new LineShape();
        shape->Set(x1, y1, x2, y2);

        return shape;
    }

    void DrawOutline(HDC hdc, long x, long y) override
    {
        MoveToEx(hdc, startX, startY, NULL);
        LineTo(hdc, x, y);
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

    void DrawOutline(HDC hdc, long x, long y) override
    {
        long oppositeX = 2 * startX - x;
        long oppositeY = 2 * startY - y;

        long left = (oppositeX < x) ? oppositeX : x;
        long top = (oppositeY < y) ? oppositeY : y;
        long right = (oppositeX > x) ? oppositeX : x;
        long bottom = (oppositeY > y) ? oppositeY : y;

        Rectangle(hdc, left, top, right, bottom);
    }

public:
    using ShapeEditor::ShapeEditor;
};

class EllipseEditor : public ShapeEditor
{
protected:
    Shape* CreateShape(long x1, long y1, long x2, long y2) override
    {
        if (x1 > x2) {
            long temp = x1;
            x1 = x2;
            x2 = temp;
        }

        if (y1 > y2) {
            long temp = y1;
            y1 = y2;
            y2 = temp;
        }

        Shape* shape = new EllipseShape(true, RGB(0, 255, 255));
        shape->Set(x1, y1, x2, y2);

        return shape;
    }

    void DrawOutline(HDC hdc, long x, long y) override
    {
        long left = (startX < x) ? startX : x;
        long top = (startY < y) ? startY : y;
        long right = (startX > x) ? startX : x;
        long bottom = (startY > y) ? startY : y;

        Ellipse(hdc, left, top, right, bottom);
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
    SetWindowTextW(hwnd, L"Графічний редактор - Точка");
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartLineEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new LineEditor(pcshape, &shapeCount, hwnd);
    SetWindowTextW(hwnd, L"Графічний редактор - Лінія");
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartRectEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new RectEditor(pcshape, &shapeCount, hwnd);
    SetWindowTextW(hwnd, L"Графічний редактор - Прямокутник");
    InvalidateRect(hwnd, NULL, TRUE);
}

void ShapeObjectsEditor::StartEllipseEditor(HWND hwnd)
{
    delete currentEditor;
    currentEditor = new EllipseEditor(pcshape, &shapeCount, hwnd);
    SetWindowTextW(hwnd, L"Графічний редактор - Еліпс");
    InvalidateRect(hwnd, NULL, TRUE);
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
    } else {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
}
