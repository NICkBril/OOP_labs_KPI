#include "cube_shape.h"

#include <algorithm>

void CubeShape::Show(HDC hdc)
{
    long x1 = xs1, y1 = ys1, x2 = xs2, y2 = ys2;

    long left = (std::min)(x1, x2);
    long top = (std::min)(y1, y2);
    long right = (std::max)(x1, x2);
    long bottom = (std::max)(y1, y2);

    long depth = (std::min)(right - left, bottom - top) / 3;


    // передня грань
    Set(left, top + depth, right - depth, bottom);
    RectShape::Show(hdc);

    // задня грань
    Set(left + depth, top, right, bottom - depth);
    RectShape::Show(hdc);

    // чотири ребра, з'днання
    Set(left, top + depth, left + depth, top);
    LineShape::Show(hdc);

    Set(right - depth, top + depth, right, top);
    LineShape::Show(hdc);

    Set(left, bottom, left + depth, bottom - depth);
    LineShape::Show(hdc);

    Set(right - depth, bottom, right, bottom - depth);
    LineShape::Show(hdc);


    Set(x1, y1, x2, y2);
}

Shape* CubeShape::Clone() const
{
    return new CubeShape(*this);
}