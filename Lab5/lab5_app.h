#pragma once

#include <windows.h>
#include <commctrl.h>

#include "my_table.h"

class Shape;

class Lab5App
{
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HWND m_toolbar;
    HIMAGELIST m_imageList;
    HMENU m_objectsMenu;
    HMENU m_windowMenu;
    int m_currentTool;

    MyTable m_table;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool OnCreate();
    void OnSize(int width);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(NMHDR* notification);
    void OnInitMenuPopup(HMENU menu);

    INT_PTR OnAboutDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void OnShapeAdded(const Shape& shape);
    void OnTableSelect(int index);
    void OnTableDelete(int index);
    void OnLoad();
    void RefreshTable();

    void RegisterShapes();

    HBITMAP CreateToolbarImage(int index);
    const wchar_t* GetToolTip(int commandId) const;

public:
    explicit Lab5App(HINSTANCE hInstance);
    ~Lab5App();

    int Run(int nCmdShow);
};