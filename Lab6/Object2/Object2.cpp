#include "framework.h"
#include "messages.h"

#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>

namespace
{
    const wchar_t CLASS_NAME[] = L"OBJECT2";
    const wchar_t TITLE[] = L"Object2 - vector of numbers";

    const int ROWS_PER_COLUMN = 20;
    const int COLUMN_WIDTH = 150;
    const int LINE_HEIGHT = 20;
}

HWND hWndManager = NULL;
std::vector<double> data;
double minValue = 0.0;
double maxValue = 0.0;


void CreateVector(const Params& p)
{
    data.clear();

    minValue = p.minValue;
    maxValue = p.maxValue;

    for (int i = 0; i < p.n; i++) {
        double r = (double)rand() / RAND_MAX;
        data.push_back(p.minValue + (p.maxValue - p.minValue) * r);
    }
}


bool PutTextToClipboard(HWND hWnd, const std::string& text)
{
    HGLOBAL hglb = GlobalAlloc(GHND, text.size() + 1);

    if (hglb == NULL) {
        return false;
    }

    char* p = (char*)GlobalLock(hglb);
    memcpy(p, text.c_str(), text.size() + 1);
    GlobalUnlock(hglb);

    if (!OpenClipboard(hWnd)) {
        GlobalFree(hglb);
        return false;
    }

    EmptyClipboard();
    
    if (SetClipboardData(CF_TEXT, hglb) == NULL) {
        GlobalFree(hglb);
        CloseClipboard();
        return false;
    }

    CloseClipboard();

    return true;
}

bool SendDataToClipboard(HWND hWnd)
{
    std::string text = "x\ty\r\n";

    for (size_t i = 0; i < data.size(); i++) {
        char line[64];
        sprintf_s(line, "%d\t%.4f\r\n", (int)(i + 1), data[i]);
        text += line;
    }

    return PutTextToClipboard(hWnd, text);
}

// приймаємо параметри від Lab6
void OnCopyData(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND sender = (HWND)wParam;
    COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;

    LPARAM result = 0;

    if (cds->dwData == DATA_PARAMS && cds->cbData == sizeof(Params)) {
        Params p = *(Params*)cds->lpData;

        CreateVector(p);
        InvalidateRect(hWnd, NULL, TRUE);

        if (SendDataToClipboard(hWnd)) {
            result = 1;
        }
    }

    if (sender != NULL) {
        PostMessageW(sender, WM_OBJECT_DONE, (WPARAM)hWnd, result);
    }
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    wchar_t text[100];

    if (data.empty()) {
        TextOutW(hdc, 10, 10, L"Waiting for data...", 19);
    } else {
        swprintf_s(
            text, L"Vector: n = %d,   range [%.2f; %.2f]",
            (int)data.size(), minValue, maxValue
        );
        TextOutW(hdc, 10, 10, text, (int)wcslen(text));

        for (size_t i = 0; i < data.size(); i++) {
            int column = (int)i / ROWS_PER_COLUMN;
            int row = (int)i % ROWS_PER_COLUMN;

            int x = 10 + column * COLUMN_WIDTH;
            int y = 45 + row * LINE_HEIGHT;

            swprintf_s(text, L"%3d:   %.4f", (int)(i + 1), data[i]);
            TextOutW(hdc, x, y, text, (int)wcslen(text));
        }
    }

    EndPaint(hWnd, &ps);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        if (hWndManager != NULL) {
            PostMessageW(hWndManager, WM_OBJECT_READY, (WPARAM)hWnd, 0);
        } else {
            // запуск без Lab6: тестові дані для перевірки 
            Params p = { 20, 0.0, 100.0 };
            CreateVector(p);
            SendDataToClipboard(hWnd);
        }
        break;

    case WM_COPYDATA:
        OnCopyData(hWnd, wParam, lParam);
        return TRUE;

    case WM_PAINT:
        OnPaint(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    srand((unsigned)time(NULL));

    if (lpCmdLine != NULL && lpCmdLine[0] != 0) {
        hWndManager = (HWND)(ULONG_PTR)wcstoull(lpCmdLine, NULL, 10);
    }

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

    HWND hWnd = CreateWindowW(
        CLASS_NAME, TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}