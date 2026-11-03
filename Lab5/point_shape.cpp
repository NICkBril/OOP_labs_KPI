#include "point_shape.h"

void PointShape::Show(HDC hdc)
{
    if (selected) {
        HPEN pen = CreateOutlinePen();
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(
            hdc, GetStockObject(NULL_BRUSH)
        );

        Rectangle(hdc, xs1 - 4, ys1 - 4, xs1 + 5, ys1 + 5);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    SetPixel(hdc, xs1, ys1, RGB(0, 0, 0));
}

Shape* PointShape::Clone() const
{
    return new PointShape(*this);
}

const char* PointShape::ClassName() const
{
    return "PointShape";
}

const wchar_t* PointShape::Title() const
{
    return L"Point";
}