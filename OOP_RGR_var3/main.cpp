#include <windows.h>
#include <commdlg.h>

#include <string>

#include "TableView.h"

#pragma comment(lib, "Comdlg32.lib")

namespace
{
    const wchar_t CLASS_NAME[] = L"TableViewerWindow";
    const wchar_t APP_TITLE[] = L"Table viewer";

    const int IDM_OPEN = 1001;
    const int IDM_SAVE = 1002;
    const int IDM_SAVEAS = 1003;
    const int IDM_EXIT = 1004;
}

TableViewClass table;
wchar_t currentFile[MAX_PATH] = L"";

void UpdateTitle(HWND hwnd)
{
    std::wstring title = APP_TITLE;

    if (currentFile[0] != 0) {
        title += L" - ";
        title += currentFile;
    }

    SetWindowTextW(hwnd, title.c_str());
}

void OnOpen(HWND hwnd)
{
    wchar_t path[MAX_PATH] = L"";

    OPENFILENAMEW ofn = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text files (*.txt;*.tsv)\0*.txt;*.tsv\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    if (!GetOpenFileNameW(&ofn)) {
        return;
    }

    if (!table.Load(hwnd, path)) {
        MessageBoxW(hwnd, L"Unable to read the table from the file",
            L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    wcscpy_s(currentFile, path);
    UpdateTitle(hwnd);
}

void OnSaveAs(HWND hwnd)
{
    if (table.IsEmpty()) {
        MessageBoxW(hwnd, L"There is no table for saving", L"Save", MB_OK);
        return;
    }

    wchar_t path[MAX_PATH];
    wcscpy_s(path, currentFile);

    OPENFILENAMEW ofn = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text files (*.txt;*.tsv)\0*.txt;*.tsv\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = L"txt";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

    if (!GetSaveFileNameW(&ofn)) {
        return;
    }

    if (!table.Save(path)) {
        MessageBoxW(hwnd, L"Unable to save the file",
            L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    wcscpy_s(currentFile, path);
    UpdateTitle(hwnd);
}

void OnSave(HWND hwnd)
{
    if (table.IsEmpty()) {
        MessageBoxW(hwnd, L"There is no table for saving", L"Save", MB_OK);
        return;
    }

    if (currentFile[0] == 0) {
        OnSaveAs(hwnd);
        return;
    }

    if (!table.Save(currentFile)) {
        MessageBoxW(hwnd, L"Unable to save the file",
            L"Error", MB_OK | MB_ICONWARNING);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_OPEN:
            OnOpen(hwnd);
            break;

        case IDM_SAVE:
            OnSave(hwnd);
            break;

        case IDM_SAVEAS:
            OnSaveAs(hwnd);
            break;

        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;

        default:
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
        break;

    case WM_SIZE:
        table.OnSize(hwnd);
        break;

    case WM_HSCROLL:
        table.OnHScroll(hwnd, wParam);
        break;

    case WM_VSCROLL:
        table.OnVScroll(hwnd, wParam);
        break;

    case WM_MOUSEWHEEL:
        table.OnMouseWheel(hwnd, wParam);
        break;

    case WM_RBUTTONDOWN:
        table.OnRButtonDown(hwnd, lParam);
        break;

    case WM_PAINT:
        table.OnPaint(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}

HMENU CreateMainMenu()
{
    HMENU menu = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();

    AppendMenuW(fileMenu, MF_STRING, IDM_OPEN, L"Open...");
    AppendMenuW(fileMenu, MF_STRING, IDM_SAVE, L"Save");
    AppendMenuW(fileMenu, MF_STRING, IDM_SAVEAS, L"Save as...");
    AppendMenuW(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(fileMenu, MF_STRING, IDM_EXIT, L"Exit");

    AppendMenuW(menu, MF_POPUP, (UINT_PTR)fileMenu, L"File");

    return menu;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        return 0;
    }

    HWND hwnd = CreateWindowW(
        CLASS_NAME, APP_TITLE,
        WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
        CW_USEDEFAULT, 0, 1000, 600,
        NULL, CreateMainMenu(), hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}