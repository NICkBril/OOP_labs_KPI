#pragma once

#include <windows.h>

class Shape
{
protected:
    long xs1, ys1;
    long xs2, ys2;
    bool rubber;
    bool selected;

    HPEN CreateOutlinePen() const;

public:
    Shape();

    void Set(long x1, long y1, long x2, long y2);
    void Get(long& x1, long& y1, long& x2, long& y2) const;
    void SetRubber(bool value);
    void SetSelected(bool value);

    virtual void Show(HDC hdc) = 0;
    virtual Shape* Clone() const = 0;
    virtual const char* ClassName() const = 0;
    virtual const wchar_t* Title() const = 0;
    virtual ~Shape() {}
};