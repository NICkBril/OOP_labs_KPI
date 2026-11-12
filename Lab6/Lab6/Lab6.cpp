#include "framework.h"
#include "Lab6.h"
#include "messages.h"

#include <cstdio>
#include <cstdlib>
#include <cwchar>

namespace
{
    const wchar_t CLASS_NAME[] = L"LAB6";
    const wchar_t TITLE[] = L"Lab6 - manager";

    const UINT_PTR TIMER_ID = 1;
    const UINT TIMEOUT_MS = 5000;
}

HINSTANCE hInst;
HWND hMainWnd = NULL;
HWND hWndObj2 = NULL;
HWND hWndObj3 = NULL;

int state = 0;
Params params = { 0, 0.0, 0.0 };
wchar_t statusText[128] = L"Waiting. Select File -> Complete...";



void SetState(int newState, const wchar_t* text)
{
    state = newState;
    wcscpy_s(statusText, text);
    InvalidateRect(hMainWnd, NULL, TRUE);
}

void Fail(const wchar_t* text)
{
    KillTimer(hMainWnd, TIMER_ID);
    SetState(0, text);
    MessageBoxW(hMainWnd, text, L"Error", MB_OK | MB_ICONWARNING);
}

bool RunProgram(const wchar_t* name)
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    // залишаємо тільки папку, де лежить Lab6.exe
    wchar_t* slash = wcsrchr(path, L'\\');

    if (slash != NULL) {
        *(slash + 1) = 0;
    }

    wchar_t projectPath[MAX_PATH];

    swprintf_s(
        projectPath,
        L"%s..\\%s\\Debug\\%s.exe",
        path,
        name,
        name
    );

    wchar_t command[2 * MAX_PATH];

    swprintf_s(
        command,
        L"\"%s\" %llu",
        projectPath, (unsigned long long)(ULONG_PTR)hMainWnd
    );

    STARTUPINFOW si = {};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = {};

    if (!CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return false;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    SetTimer(hMainWnd, TIMER_ID, TIMEOUT_MS, NULL);

    return true;
}

void CloseProgram(const wchar_t* className)
{
    HWND hWnd = FindWindowW(className, NULL);

    if (hWnd != NULL) {
        PostMessageW(hWnd, WM_CLOSE, 0, 0);
    }
}



void SendToObject2()
{
    SetState(4, L"State 4: Parameters sent to Object2, waiting for result");

    COPYDATASTRUCT cds;
    cds.dwData = DATA_PARAMS;
    cds.cbData = sizeof(Params);
    cds.lpData = &params;

    SendMessageW(hWndObj2, WM_COPYDATA, (WPARAM)hMainWnd, (LPARAM)&cds);
}

void SendToObject3()
{
    SetState(8, L"State 8: Request sent to Object3, waiting for response");

    PostMessageW(hWndObj3, WM_DO_GRAPH, (WPARAM)hMainWnd, params.n);
}

void StartObject3()
{
    hWndObj3 = FindWindowW(L"OBJECT3", NULL);

    if (hWndObj3 != NULL) {
        SetState(7, L"State 7: Object3 is already running");
        SendToObject3();
    } else {
        if (!RunProgram(L"Object3")) {
            Fail(L"Cannot open Object3.exe");
            return;
        }

        SetState(6, L"State 6: Object3 is running, waiting for it to be ready");
    }
}

void ArrangeWindows()
{
    RECT work;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);

    int width = work.right - work.left;
    int height = work.bottom - work.top;
    int topHeight = height * 70 / 100;

    ShowWindow(hWndObj2, SW_RESTORE);
    ShowWindow(hWndObj3, SW_RESTORE);

    SetWindowPos(
        hWndObj2, HWND_TOP,
        work.left, work.top, width / 2, topHeight, SWP_SHOWWINDOW
    );

    SetWindowPos(
        hWndObj3, HWND_TOP,
        work.left + width / 2, work.top, width - width / 2, topHeight, SWP_SHOWWINDOW
    );

    SetWindowPos(
        hMainWnd, HWND_TOP,
        work.left, work.top + topHeight, width, height - topHeight, SWP_SHOWWINDOW
    );
}


void StartWork()
{
    SetState(1, L"State 1: Parameters entered");

    hWndObj2 = FindWindowW(L"OBJECT2", NULL);

    if (hWndObj2 != NULL) {
        SetState(3, L"State 3: Object2 is already running");
        SendToObject2();
    } else {
        if (!RunProgram(L"Object2")) {
            Fail(L"Cannot open Object2.exe");
            return;
        }

        SetState(2, L"State 2: Object2 is running, waiting for it to be ready");
    }
}

void OnObjectReady(HWND from)
{
    KillTimer(hMainWnd, TIMER_ID);

    if (state == 2) {
        hWndObj2 = from;
        SetState(3, L"State 3: HWND Object2 received");
        SendToObject2();
    } else if (state == 6) {
        hWndObj3 = from;
        SetState(7, L"State 7: HWND Object3 received");
        SendToObject3();
    }
}

void OnObjectDone(HWND from, LPARAM success)
{
    if (state == 4) {
        if (!success) {
            Fail(L"Object2 was unable to create or save the data");
            return;
        }

        SetState(5, L"State 5: Object2 wrote data to the clipboard");
        StartObject3();
    } else if (state == 8) {
        if (!success) {
            Fail(L"Object3 was unable to read the data or generate a graphic");
            return;
        }

        SetState(9, L"State 9: Object3 generated a graphic");
        ArrangeWindows();
        SetState(0, L"The task is complete. You can run it again (File -> Run...)");
    }
}



bool ReadDouble(HWND hDlg, int id, double& value)
{
    wchar_t text[64];
    GetDlgItemTextW(hDlg, id, text, 64);

    for (int i = 0; text[i] != 0; i++) {
        if (text[i] == L',') {
            text[i] = L'.';
        }
    }

    wchar_t* end;
    value = wcstod(text, &end);

    return end != text && *end == 0;
}

INT_PTR CALLBACK ParamsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemTextW(hDlg, IDC_EDIT_N, L"20");
        SetDlgItemTextW(hDlg, IDC_EDIT_MIN, L"0");
        SetDlgItemTextW(hDlg, IDC_EDIT_MAX, L"100");
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            BOOL ok;
            int n = GetDlgItemInt(hDlg, IDC_EDIT_N, &ok, FALSE);

            if (!ok || n < 1 || n > 100) {
                MessageBoxW(hDlg, L"n must be an integer between 1 and 100",
                    L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            double minValue, maxValue;

            if (!ReadDouble(hDlg, IDC_EDIT_MIN, minValue) ||
                !ReadDouble(hDlg, IDC_EDIT_MAX, maxValue)) {
                MessageBoxW(hDlg, L"Min and Max must be numbers",
                    L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (minValue > maxValue) {
                MessageBoxW(hDlg, L"Min must be less than Max",
                    L"Error", MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            params.n = n;
            params.minValue = minValue;
            params.maxValue = maxValue;

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

void OnRun(HWND hWnd)
{
    if (state != 0) {
        MessageBoxW(hWnd, L"Work is still in progress", L"Lab6", MB_OK);
        return;
    }

    if (DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_PARAMS), hWnd, ParamsDlgProc) == IDOK) {
        StartWork();
    }
}




void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    wchar_t text[200];

    swprintf_s(text, L"Current State: %d", state);
    TextOutW(hdc, 10, 10, text, (int)wcslen(text));

    TextOutW(hdc, 10, 35, statusText, (int)wcslen(statusText));

    if (params.n > 0) {
        swprintf_s(
            text, L"Parameters: n = %d, Min = %.2f, Max = %.2f",
            params.n, params.minValue, params.maxValue
        );
        TextOutW(hdc, 10, 60, text, (int)wcslen(text));
    }

    EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_RUN:
            OnRun(hWnd);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;

    case WM_OBJECT_READY:
        OnObjectReady((HWND)wParam);
        break;

    case WM_OBJECT_DONE:
        OnObjectDone((HWND)wParam, lParam);
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            Fail(L"The program did not respond in time");
        }
        break;

    case WM_PAINT:
        OnPaint(hWnd);
        break;

    case WM_DESTROY:
        CloseProgram(L"OBJECT2");
        CloseProgram(L"OBJECT3");
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    hInst = hInstance;

    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_LAB6));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB6);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        return 0;
    }

    hMainWnd = CreateWindowW(
        CLASS_NAME, TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 700, 300,
        NULL, NULL, hInstance, NULL
    );

    if (hMainWnd == NULL) {
        return 0;
    }

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    MSG msg;

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
