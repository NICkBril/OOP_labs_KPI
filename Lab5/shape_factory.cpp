#include "shape_factory.h"
#include "shape.h"

ShapeFactory::~ShapeFactory()
{
    for (auto& item : prototypes) {
        delete item.second;
    }
}

void ShapeFactory::Register(Shape* prototype)
{
    if (prototype == nullptr) {
        return;
    }

    std::string name = prototype->ClassName();

    auto found = prototypes.find(name);

    if (found != prototypes.end()) {
        delete found->second;
    }

    prototypes[name] = prototype;
}

Shape* ShapeFactory::Create(const char* className) const
{
    auto found = prototypes.find(className);

    if (found == prototypes.end()) {
        return nullptr;
    }

    return found->second->Clone();
}