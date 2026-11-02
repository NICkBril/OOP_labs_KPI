#include "my_editor.h"
#include "shape.h"
#include "shape_factory.h"

#include <cstdio>

namespace {
    const char DATA_FILE[] = "shapes.txt";

    void WriteShape(FILE* file, const Shape& shape)
    {
        long x1, y1, x2, y2;
        shape.Get(x1, y1, x2, y2);

        fprintf(
            file, "%s\t%ld\t%ld\t%ld\t%ld\n",
            shape.ClassName(), x1, y1, x2, y2
        );
    }

    void AppendShape(const Shape& shape)
    {
        FILE* fout = nullptr;

        if (fopen_s(&fout, DATA_FILE, "at") == 0 && fout != nullptr) {
            WriteShape(fout, shape);
            fclose(fout);
        }
    }
}

MyEditor::MyEditor()
    : shapeCount(0),
    selected(-1),
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

void MyEditor::SetShapeAddedHandler(ShapeHandler handler)
{
    onShapeAdded = handler;
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

        AppendShape(*shape);

        if (onShapeAdded) {
            onShapeAdded(*shape);
        }
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

int MyEditor::GetCount() const
{
    return shapeCount;
}

const Shape* MyEditor::GetShape(int index) const
{
    if (index < 0 || index >= shapeCount) {
        return nullptr;
    }

    return pshape[index];
}

void MyEditor::Select(int index)
{
    if (selected >= 0 && selected < shapeCount) {
        pshape[selected]->SetSelected(false);
    }

    if (index >= 0 && index < shapeCount) {
        selected = index;
        pshape[selected]->SetSelected(true);
    } else {
        selected = -1;
    }
}

bool MyEditor::Remove(int index)
{
    if (index < 0 || index >= shapeCount) {
        return false;
    }

    delete pshape[index];

    for (int i = index; i < shapeCount - 1; ++i) {
        pshape[i] = pshape[i + 1];
    }

    pshape[--shapeCount] = nullptr;

    if (selected == index) {
        selected = -1;
    } else if (selected > index) {
        --selected;
    }

    Save();

    return true;
}

bool MyEditor::Save() const
{
    FILE* fout = nullptr;

    if (fopen_s(&fout, DATA_FILE, "wt") != 0 || fout == nullptr) {
        return false;
    }

    for (int i = 0; i < shapeCount; ++i) {
        WriteShape(fout, *pshape[i]);
    }

    fclose(fout);

    return true;
}

bool MyEditor::Load()
{
    FILE* fin = nullptr;

    if (fopen_s(&fin, DATA_FILE, "rt") != 0 || fin == nullptr) {
        return false;
    }

    for (int i = 0; i < shapeCount; ++i) {
        delete pshape[i];
        pshape[i] = nullptr;
    }

    shapeCount = 0;
    selected = -1;

    char buf[256];

    while (shapeCount < MAX_SHAPES && fgets(buf, sizeof(buf), fin)) {
        char name[64];
        long x1, y1, x2, y2;

        if (sscanf_s(buf, "%63s %ld %ld %ld %ld",
            name, (unsigned)_countof(name), &x1, &y1, &x2, &y2
        ) != 5) {
            continue;
        }

        Shape* shape = ShapeFactory::getInstance().Create(name);

        if (shape != nullptr) {
            shape->Set(x1, y1, x2, y2);
            pshape[shapeCount++] = shape;
        }
    }

    fclose(fin);

    return true;
}