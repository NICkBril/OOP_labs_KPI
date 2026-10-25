#pragma once

#include "shape.h"

class EllipseShape : public virtual Shape
{
public:
    void Show(HDC hdc) override;
    Shape* Clone() const override;
};