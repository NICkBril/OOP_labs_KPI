#include "rect_shape.h"

#include <algorithm>

void RectShape::Show(HDC hdc)
{
    long left = (std::min)(xs1, xs2);
    long top = (std::min)(ys1, ys2);
    long right = (std::max)(xs1, xs2);
    long bottom = (std::max)(ys1, ys2);

    HPEN pen = CreateOutlinePen();
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(
        hdc, GetStockObject(NULL_BRUSH)
    );

    Rectangle(hdc, left, top, right, bottom);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

Shape* RectShape::Clone() const
{
    return new RectShape(*this);
}