#include "ellipse_shape.h"

void EllipseShape::Show(HDC hdc)
{
    long radiusX = (xs2 >= xs1) ? xs2 - xs1 : xs1 - xs2;
    long radiusY = (ys2 >= ys1) ? ys2 - ys1 : ys1 - ys2;

    HPEN pen = CreateOutlinePen();
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    HBRUSH brush = NULL;
    HBRUSH oldBrush;

    if (rubber) {
        oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    } else {
        brush = CreateSolidBrush(RGB(255, 0, 255));
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
    }

    Ellipse(
        hdc,
        xs1 - radiusX, ys1 - radiusY,
        xs1 + radiusX, ys1 + radiusY
    );

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);

    DeleteObject(pen);

    if (brush != NULL) {
        DeleteObject(brush);
    }
}

Shape* EllipseShape::Clone() const
{
    return new EllipseShape(*this);
}