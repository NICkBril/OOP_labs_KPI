#pragma once

#include <windows.h>

class Shape
{
protected:
    long xs1, ys1;
    long xs2, ys2;

public:
    void Set(long x1, long y1, long x2, long y2);

    virtual void Show(HDC hdc) = 0;

    virtual ~Shape() {}
};

class PointShape : public Shape
{
public:
    void Show(HDC hdc) override;
};

class LineShape : public Shape
{
public:
    void Show(HDC hdc) override;
};

class RectShape : public Shape
{
private:
    bool filled;
    COLORREF fillColor;

public:
    RectShape(bool filled = false, COLORREF color = RGB(255, 255, 255));

    void Show(HDC hdc) override;
};

class EllipseShape : public Shape
{
private:
    bool filled;
    COLORREF fillColor;

public:
    EllipseShape(bool filled = false, COLORREF color = RGB(255, 255, 255));

    void Show(HDC hdc) override;
};