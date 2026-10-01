#include "framework.h"
#include "module1.h"
#include "Resource.h"

struct MOD1_DATA {
    wchar_t* buffer;
    int maxCount;
};

static INT_PTR CALLBACK DlgProcWork1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MOD1_DATA* pData = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pData = (MOD1_DATA*)lParam;

        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_ADDSTRING, 0, (LPARAM)L"IM-51");
        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_ADDSTRING, 0, (LPARAM)L"IM-52");
        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_ADDSTRING, 0, (LPARAM)L"IM-53");
        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_ADDSTRING, 0, (LPARAM)L"IM-54");
        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_ADDSTRING, 0, (LPARAM)L"IM-55");

        SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_SETCURSEL, 0, 0);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK) {
            if (pData && pData->buffer) {
                int selIndex = (int)SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_GETCURSEL, 0, 0);
                if (selIndex != LB_ERR) {
                    SendDlgItemMessage(hDlg, IDC_LIST_GROUPS, LB_GETTEXT, selIndex, (LPARAM)pData->buffer);
                } else {
                    pData->buffer[0] = L'\0';
                }
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

int Func_MOD1(HWND hWnd, wchar_t* outBuf, int maxCount)
{
    MOD1_DATA data = { outBuf, maxCount };
    INT_PTR res = DialogBoxParam(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_DIALOG_MOD1),
        hWnd,
        DlgProcWork1,
        (LPARAM)&data
    );
    return (res == 1) ? 1 : 0;
}

extern WCHAR g_DisplayText[256];

void MyWork1(HWND hWnd)
{
    WCHAR tempBuf[256] = { 0 };

    if (Func_MOD1(hWnd, tempBuf, 256) == 1) {
        wsprintfW(g_DisplayText, L"Selected group: %s", tempBuf);
        InvalidateRect(hWnd, NULL, TRUE);
    }
}