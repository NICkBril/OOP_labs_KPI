#pragma once

#include "shape.h"

class EllipseShape : public virtual Shape
{
public:
    void Show(HDC hdc) override;
    Shape* Clone() const override;
    const char* ClassName() const override;
    const wchar_t* Title() const override;
};