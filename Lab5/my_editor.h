#pragma once

#include <windows.h>
#include <functional>

class Shape;

class MyEditor
{
public:
    typedef std::function<void(const Shape&)> ShapeHandler;

private:
    static const int MAX_SHAPES = 104;

    Shape* pshape[MAX_SHAPES];
    int shapeCount;
    int selected;

    Shape* pcurrent;

    long startX, startY;
    long lastX, lastY;
    bool drawing;

    ShapeHandler onShapeAdded;

    MyEditor();

    void Draw(HDC hdc, const RECT& clientRect);

public:
    ~MyEditor();

    MyEditor(const MyEditor&) = delete;
    MyEditor& operator=(const MyEditor&) = delete;

    static MyEditor& getInstance()
    {
        static MyEditor instance;
        return instance;
    }

    void SetShapeAddedHandler(ShapeHandler handler);

    void Start(Shape* shape);
    void OnLBdown(HWND hwnd);
    void OnLBup(HWND hwnd);
    void OnMouseMove(HWND hwnd);
    void OnPaint(HWND hwnd);

    int GetCount() const;
    const Shape* GetShape(int index) const;

    void Select(int index);
    bool Remove(int index);

    bool Save() const;
    bool Load();
};