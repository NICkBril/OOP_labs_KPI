#include "line_shape.h"

void LineShape::Show(HDC hdc)
{
    HPEN pen = CreateOutlinePen();
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, xs1, ys1, NULL);
    LineTo(hdc, xs2, ys2);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

Shape* LineShape::Clone() const
{
    return new LineShape(*this);
}