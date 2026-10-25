#include "line_oo_shape.h"

namespace {
    const long CIRCLE_RADIUS = 6;
}

void LineOOShape::Show(HDC hdc)
{
    long x1 = xs1, y1 = ys1, x2 = xs2, y2 = ys2;

    LineShape::Show(hdc);

    Set(x1, y1, x1 + CIRCLE_RADIUS, y1 + CIRCLE_RADIUS);
    EllipseShape::Show(hdc);

    Set(x2, y2, x2 + CIRCLE_RADIUS, y2 + CIRCLE_RADIUS);
    EllipseShape::Show(hdc);


    Set(x1, y1, x2, y2);
}

Shape* LineOOShape::Clone() const
{
    return new LineOOShape(*this);
}