#pragma once

#include <windows.h>
#include <commctrl.h>

class Lab4App
{
private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HWND m_toolbar;
    HIMAGELIST m_imageList;
    HMENU m_objectsMenu;
    int m_currentTool;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool OnCreate();
    void OnSize(int width);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(NMHDR* notification);
    void OnInitMenuPopup(HMENU menu);

    INT_PTR OnAboutDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HBITMAP CreateToolbarImage(int index);
    const wchar_t* GetToolTip(int commandId) const;

public:
    explicit Lab4App(HINSTANCE hInstance);
    ~Lab4App();

    int Run(int nCmdShow);
};