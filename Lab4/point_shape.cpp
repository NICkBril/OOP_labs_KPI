#include "point_shape.h"

void PointShape::Show(HDC hdc)
{
    SetPixel(hdc, xs1, ys1, RGB(0, 0, 0));
}

Shape* PointShape::Clone() const
{
    return new PointShape(*this);
}