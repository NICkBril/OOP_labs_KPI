#pragma once

#include <windows.h>
#include <string>
#include <vector>

class TableViewClass
{
private:
    typedef std::vector<std::wstring> Row;

    std::vector<std::wstring> headers;
    std::vector<Row> rows;
    std::vector<int> order;
    std::vector<int> colWidths;

    int sortColumn;
    int sortState;

    int scrollX, scrollY;
    int rowHeight, headerHeight;


    void Measure(HWND hwnd);
    void ApplySort();
    void UpdateScrollBars(HWND hwnd);
    void ScrollTo(HWND hwnd, int x, int y);
    void Scroll(HWND hwnd, int bar, WPARAM wParam);
    void Draw(HDC hdc, const RECT& rc) const;

    int ContentWidth() const;
    int ContentHeight() const;

public:
    TableViewClass();

    bool Load(HWND hwnd, const wchar_t* path);
    bool Save(const wchar_t* path) const;
    bool IsEmpty() const;

    void OnPaint(HWND hwnd);
    void OnSize(HWND hwnd);
    void OnHScroll(HWND hwnd, WPARAM wParam);
    void OnVScroll(HWND hwnd, WPARAM wParam);
    void OnMouseWheel(HWND hwnd, WPARAM wParam);
    void OnRButtonDown(HWND hwnd, LPARAM lParam);
};