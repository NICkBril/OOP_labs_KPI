#pragma once

#include <map>
#include <string>

class Shape;

class ShapeFactory
{
private:
    std::map<std::string, Shape*> prototypes;

    ShapeFactory() {}
    ShapeFactory(const ShapeFactory&) = delete;
    ShapeFactory& operator=(const ShapeFactory&) = delete;

public:
    ~ShapeFactory();

    static ShapeFactory& getInstance()
    {
        static ShapeFactory instance;
        return instance;
    }

    void Register(Shape* prototype);
    Shape* Create(const char* className) const;
};