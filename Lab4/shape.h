#pragma once

#include <windows.h>

class Shape
{
protected:
    long xs1, ys1;
    long xs2, ys2;
    bool rubber;

    HPEN CreateOutlinePen() const;

public:
    Shape();

    void Set(long x1, long y1, long x2, long y2);
    void SetRubber(bool value);

    virtual void Show(HDC hdc) = 0;
    virtual Shape* Clone() const = 0;
    virtual ~Shape() {}
};