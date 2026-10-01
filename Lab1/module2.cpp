#include "framework.h"
#include "module2.h"
#include "Resource.h"

struct MOD2_DATA {
    wchar_t* buffer;
    int maxCount;
};

static INT_PTR CALLBACK DlgProcWork2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MOD2_DATA* pData = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pData = (MOD2_DATA*)lParam;
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK) {
            if (pData && pData->buffer) {
                GetDlgItemTextW(hDlg, IDC_EDIT_TEXT, pData->buffer, pData->maxCount);
            }
            EndDialog(hDlg, 1);
            return (INT_PTR)TRUE;
        } else if (wmId == IDCANCEL) {
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
        break;
    }
    }
    return (INT_PTR)FALSE;
}


int Func_MOD2(HWND hWnd, wchar_t* outBuf, int maxCount)
{
    MOD2_DATA data = { outBuf, maxCount };
    INT_PTR res = DialogBoxParam(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_DIALOG_MOD2),
        hWnd,
        DlgProcWork2,
        (LPARAM)&data
    );

    return (res == 1) ? 1 : 0;
}

extern WCHAR g_DisplayText[256];

void MyWork2(HWND hWnd)
{
    WCHAR tempBuf[256] = { 0 };

    if (Func_MOD2(hWnd, tempBuf, 256) == 1) {
        wsprintfW(g_DisplayText, L"Your text: %s", tempBuf);
        InvalidateRect(hWnd, NULL, TRUE);
    }
}