#pragma once

#include <windows.h>
#include <functional>
#include <string>
#include <vector>

class MyTable
{
public:
    typedef std::function<void(int)> IndexHandler;

private:
    struct Row
    {
        std::wstring name;
        long x1, y1, x2, y2;
    };

    HWND m_hDlg;
    HWND m_hList;
    HWND m_hDelete;

    std::vector<Row> m_rows;

    IndexHandler m_onSelect;
    IndexHandler m_onDelete;

    bool m_updating;

    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    INT_PTR HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    void OnInitDialog(HWND hDlg);
    void OnSize(int width, int height);
    void OnNotify(NMHDR* notification);
    void OnDeleteRow();

    void InsertRow(int index, const Row& row);
    int GetSelected() const;

public:
    MyTable();
    ~MyTable();

    MyTable(const MyTable&) = delete;
    MyTable& operator=(const MyTable&) = delete;

    void SetSelectHandler(IndexHandler handler);
    void SetDeleteHandler(IndexHandler handler);

    void Open(HINSTANCE hInstance, HWND parent);
    void Close();
    void Toggle(HINSTANCE hInstance, HWND parent);
    bool IsOpen() const;

    bool ProcessMessage(MSG* message) const;

    void Add(const wchar_t* name, long x1, long y1, long x2, long y2);
    void RemoveAt(int index);
    void Clear();
};