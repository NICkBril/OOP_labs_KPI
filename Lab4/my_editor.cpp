#include "my_editor.h"
#include "shape.h"

MyEditor::MyEditor()
    : shapeCount(0),
    pcurrent(nullptr),
    startX(0),
    startY(0),
    lastX(0),
    lastY(0),
    drawing(false)
{
    for (int i = 0; i < MAX_SHAPES; ++i) {
        pshape[i] = nullptr;
    }
}

MyEditor::~MyEditor()
{
    delete pcurrent;

    for (int i = 0; i < shapeCount; ++i) {
        delete pshape[i];
    }
}

void MyEditor::Start(Shape* shape)
{
    delete pcurrent;

    pcurrent = shape;
    drawing = false;

    if (pcurrent != nullptr) {
        pcurrent->SetRubber(true);
    }
}

void MyEditor::OnLBdown(HWND hwnd)
{
    if (pcurrent == nullptr || drawing) {
        return;
    }

    POINT point;
    GetCursorPos(&point);
    ScreenToClient(hwnd, &point);

    startX = lastX = point.x;
    startY = lastY = point.y;

    drawing = true;
    SetCapture(hwnd);
}

void MyEditor::OnLBup(HWND hwnd)
{
    if (!drawing) {
        return;
    }

    POINT point;
    GetCursorPos(&point);
    ScreenToClient(hwnd, &point);

    drawing = false;
    ReleaseCapture();

    if (shapeCount < MAX_SHAPES) {
        Shape* shape = pcurrent->Clone();

        shape->SetRubber(false);
        shape->Set(startX, startY, point.x, point.y);

        pshape[shapeCount++] = shape;
    }

    InvalidateRect(hwnd, NULL, FALSE);
}

void MyEditor::OnMouseMove(HWND hwnd)
{
    if (!drawing) {
        return;
    }

    POINT point;
    GetCursorPos(&point);
    ScreenToClient(hwnd, &point);

    lastX = point.x;
    lastY = point.y;

    InvalidateRect(hwnd, NULL, FALSE);
}

void MyEditor::Draw(HDC hdc, const RECT& clientRect)
{
    FillRect(hdc, &clientRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

    for (int i = 0; i < shapeCount; ++i) {
        pshape[i]->Show(hdc);
    }

    if (drawing && pcurrent != nullptr) {
        pcurrent->Set(startX, startY, lastX, lastY);
        pcurrent->Show(hdc);
    }
}

void MyEditor::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    if (width > 0 && height > 0) {
        HDC memoryDC = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, width, height);

        if (memoryDC != NULL && bitmap != NULL) {
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);

            Draw(memoryDC, clientRect);

            BitBlt(hdc, 0, 0, width, height, memoryDC, 0, 0, SRCCOPY);

            SelectObject(memoryDC, oldBitmap);
        } else {
            Draw(hdc, clientRect);
        }

        if (bitmap != NULL) {
            DeleteObject(bitmap);
        }

        if (memoryDC != NULL) {
            DeleteDC(memoryDC);
        }
    }

    EndPaint(hwnd, &ps);
}