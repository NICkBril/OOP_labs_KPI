#include "shape.h"

Shape::Shape()
    : xs1(0), ys1(0), xs2(0), ys2(0), rubber(false)
{
}

void Shape::Set(long x1, long y1, long x2, long y2)
{
    xs1 = x1;
    ys1 = y1;
    xs2 = x2;
    ys2 = y2;
}

void Shape::SetRubber(bool value)
{
    rubber = value;
}

HPEN Shape::CreateOutlinePen() const
{
    return CreatePen(rubber ? PS_DOT : PS_SOLID, 1, RGB(0, 0, 0));
}