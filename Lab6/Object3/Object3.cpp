#include "framework.h"
#include "messages.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cwchar>

namespace
{
    const wchar_t CLASS_NAME[] = L"OBJECT3";
    const wchar_t TITLE[] = L"Object3 - graphic";
}

HWND hWndManager = NULL;
std::vector<double> values;

std::string GetTextFromClipboard(HWND hWnd)
{
    std::string result;

    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        return result;
    }

    if (!OpenClipboard(hWnd)) {
        return result;
    }

    HGLOBAL hglb = GetClipboardData(CF_TEXT);

    if (hglb != NULL) {
        const char* text = (const char*)GlobalLock(hglb);

        if (text != NULL) {
            result = text;
            GlobalUnlock(hglb);
        }
    }

    CloseClipboard();

    return result;
}


void ParseData(const std::string& text)
{
    values.clear();

    std::istringstream in(text);
    std::string line;

    while (std::getline(in, line)) {
        int index;
        double value;

        if (sscanf_s(line.c_str(), "%d %lf", &index, &value) == 2) {
            values.push_back(value);
        }
    }
}


void OnDoGraph(HWND hWnd, HWND sender)
{
    ParseData(GetTextFromClipboard(hWnd));

    InvalidateRect(hWnd, NULL, TRUE);

    if (sender != NULL) {
        PostMessageW(
            sender, WM_OBJECT_DONE,
            (WPARAM)hWnd, values.empty() ? 0 : 1
        );
    }
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    if (values.empty()) {
        TextOutW(hdc, 10, 10, L"No data for the graphic", 23);
        EndPaint(hWnd, &ps);
        return;
    }

    RECT rc;
    GetClientRect(hWnd, &rc);

    int plotLeft = 70;
    int plotRight = rc.right - 30;
    int plotTop = 40;
    int plotBottom = rc.bottom - 50;

    if (plotRight - plotLeft < 50 || plotBottom - plotTop < 50) {
        EndPaint(hWnd, &ps);
        return;
    }

    int n = (int)values.size();

    double yMin = *std::min_element(values.begin(), values.end());
    double yMax = *std::max_element(values.begin(), values.end());

    if (yMax - yMin < 1e-9) {
        yMin -= 1.0;
        yMax += 1.0;
    }

    double xMin = 1.0;
    double xMax = (n == 1) ? 2.0 : (double)n;


    auto toX = [&](double x) {
        return plotLeft + (int)((x - xMin) / (xMax - xMin) * (plotRight - plotLeft));
        };

    auto toY = [&](double y) {
        return plotBottom - (int)((y - yMin) / (yMax - yMin) * (plotBottom - plotTop));
        };

    SetBkMode(hdc, TRANSPARENT);

    wchar_t text[64];

    swprintf_s(text, L"y = f(x), n = %d", n);
    TextOutW(hdc, plotLeft, 10, text, (int)wcslen(text));

    // сітка + позначення для Y
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

    SetTextAlign(hdc, TA_RIGHT | TA_TOP);

    for (int k = 0; k <= 5; k++) {
        double value = yMin + (yMax - yMin) * k / 5.0;
        int y = toY(value);

        MoveToEx(hdc, plotLeft, y, NULL);
        LineTo(hdc, plotRight, y);

        swprintf_s(text, L"%.2f", value);
        TextOutW(hdc, plotLeft - 8, y - 8, text, (int)wcslen(text));
    }

    // сітка + позначення для X
    SetTextAlign(hdc, TA_CENTER | TA_TOP);

    int step = n / 10;

    if (step < 1) {
        step = 1;
    }

    for (int i = 1; i <= n; i++) {
        if (i == 1 || i % step == 0) {
            int x = toX(i);

            MoveToEx(hdc, x, plotTop, NULL);
            LineTo(hdc, x, plotBottom);

            swprintf_s(text, L"%d", i);
            TextOutW(hdc, x, plotBottom + 8, text, (int)wcslen(text));
        }
    }

    SetTextAlign(hdc, TA_LEFT | TA_TOP);

    // лінія y = 0
    HPEN zeroPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
    SelectObject(hdc, zeroPen);

    if (yMin < 0 && yMax > 0) {
        MoveToEx(hdc, plotLeft, toY(0), NULL);
        LineTo(hdc, plotRight, toY(0));
    }


    // осі координат
    HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    SelectObject(hdc, axisPen);

    MoveToEx(hdc, plotLeft, plotTop, NULL);
    LineTo(hdc, plotLeft, plotBottom);
    LineTo(hdc, plotRight, plotBottom);

    TextOutW(hdc, plotLeft + 6, plotTop - 18, L"y", 1);
    TextOutW(hdc, plotRight - 10, plotBottom + 25, L"x", 1);
    

    HPEN linePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
    SelectObject(hdc, linePen);

    MoveToEx(hdc, toX(1), toY(values[0]), NULL);

    for (int i = 1; i < n; i++) {
        LineTo(hdc, toX(i + 1), toY(values[i]));
    }

    if (n <= 50) {
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

        for (int i = 0; i < n; i++) {
            int x = toX(i + 1);
            int y = toY(values[i]);

            Ellipse(hdc, x - 3, y - 3, x + 4, y + 4);
        }

        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
    }

    SelectObject(hdc, oldPen);

    DeleteObject(gridPen);
    DeleteObject(zeroPen);
    DeleteObject(axisPen);
    DeleteObject(linePen);

    EndPaint(hWnd, &ps);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        if (hWndManager != NULL) {
            PostMessageW(hWndManager, WM_OBJECT_READY, (WPARAM)hWnd, 0);
        } else {
            OnDoGraph(hWnd, NULL);
        }
        break;

    case WM_DO_GRAPH:
        OnDoGraph(hWnd, (HWND)wParam);
        break;

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