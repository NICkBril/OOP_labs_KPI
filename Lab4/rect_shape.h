#pragma once

#include "shape.h"

class RectShape : public virtual Shape
{
public:
    void Show(HDC hdc) override;
    Shape* Clone() const override;
};