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

void RectShape::Show(HDC hdc) 
{
    HBRUSH oldBrush = (HBRUSH)SelectObject(
        hdc, GetStockObject(NULL_BRUSH)
    );

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Rectangle(hdc, xs1, ys1, xs2, ys2);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}

void EllipseShape::Show(HDC hdc)
{
    HBRUSH brush = CreateSolidBrush(RGB(255, 0, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, xs1, ys1, xs2, ys2);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);

    DeleteObject(pen);
    DeleteObject(brush);
}