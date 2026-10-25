#pragma once

#include "shape.h"

class PointShape : public Shape
{
public:
    void Show(HDC hdc) override;
    Shape* Clone() const override;
};