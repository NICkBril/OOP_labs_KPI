#include "shape.h"

void Shape::Set(long x1, long y1, long x2, long y2)
{
    xs1 = x1;
    ys1 = y1;
    xs2 = x2;
    ys2 = y2;
}

void PointShape::Show(HDC hdc)
{
    SetPixel(hdc, xs1, ys1, RGB(0, 0, 0));
}

void LineShape::Show(HDC hdc)
{
    MoveToEx(hdc, xs1, ys1, NULL);
    LineTo(hdc, xs2, ys2);
}

RectShape::RectShape(bool filled, COLORREF color)
{
    this->filled = filled;
    fillColor = color;
}

void RectShape::Show(HDC hdc)
{
    HBRUSH hBrush;
    HBRUSH hOldBrush;

    if (filled) {
        hBrush = CreateSolidBrush(fillColor);
    }
    else {
        hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    }


    hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    Rectangle(hdc, xs1, ys1, xs2, ys2);

    SelectObject(hdc, hOldBrush);

    if (filled) {
        DeleteObject(hBrush);
    }
}

EllipseShape::EllipseShape(bool filled, COLORREF color)
{
    this->filled = filled;
    fillColor = color;
}

void EllipseShape::Show(HDC hdc)
{
    HBRUSH hBrush;
    HBRUSH hOldBrush;

    if (filled) {
        hBrush = CreateSolidBrush(fillColor);
    } else {
        hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    }


    hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    Ellipse(hdc, xs1, ys1, xs2, ys2);

    SelectObject(hdc, hOldBrush);

    if (filled) {
        DeleteObject(hBrush);
    }
}