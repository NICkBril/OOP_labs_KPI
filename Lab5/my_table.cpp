#include "framework.h"
#include "my_table.h"

#include <commctrl.h>
#include <cstdio>

#pragma comment(lib, "Comctl32.lib")

namespace
{
    const int ID_DELETE = 2001;

    const int MARGIN = 6;
    const int BUTTON_WIDTH = 100;
    const int BUTTON_HEIGHT = 28;

    const int DIALOG_WIDTH = 440;
    const int DIALOG_HEIGHT = 340;
}

MyTable::MyTable()
    : m_hDlg(nullptr),
    m_hList(nullptr),
    m_hDelete(nullptr),
    m_updating(false)
{
}

MyTable::~MyTable()
{
    Close();
}

void MyTable::SetSelectHandler(IndexHandler handler)
{
    m_onSelect = handler;
}

void MyTable::SetDeleteHandler(IndexHandler handler)
{
    m_onDelete = handler;
}

void MyTable::Open(HINSTANCE hInstance, HWND parent)
{
    if (m_hDlg != nullptr) {
        return;
    }

    INITCOMMONCONTROLSEX controls = {};

    controls.dwSize = sizeof(INITCOMMONCONTROLSEX);
    controls.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&controls);

    alignas(4) WORD buffer[32] = {};

    DLGTEMPLATE* tmpl = reinterpret_cast<DLGTEMPLATE*>(buffer);

    tmpl->style = WS_POPUP | WS_CAPTION | WS_SYSMENU |
        WS_THICKFRAME | WS_CLIPCHILDREN | WS_VISIBLE;
    tmpl->cdit = 0;
    tmpl->cx = 100;
    tmpl->cy = 100;

    CreateDialogIndirectParamW(
        hInstance,
        tmpl,
        parent,
        DialogProc,
        reinterpret_cast<LPARAM>(this)
    );
}

void MyTable::Close()
{
    if (m_hDlg != nullptr) {
        DestroyWindow(m_hDlg);
    }
}

void MyTable::Toggle(HINSTANCE hInstance, HWND parent)
{
    if (m_hDlg != nullptr) {
        Close();
    }
    else {
        Open(hInstance, parent);
    }
}

bool MyTable::IsOpen() const
{
    return m_hDlg != nullptr;
}

bool MyTable::ProcessMessage(MSG* message) const
{
    return m_hDlg != nullptr && IsDialogMessageW(m_hDlg, message);
}

void MyTable::Add(const wchar_t* name, long x1, long y1, long x2, long y2)
{
    Row row;

    row.name = name;
    row.x1 = x1;
    row.y1 = y1;
    row.x2 = x2;
    row.y2 = y2;

    m_rows.push_back(row);

    if (m_hList != nullptr) {
        int index = static_cast<int>(m_rows.size()) - 1;

        InsertRow(index, row);
        SendMessageW(m_hList, LVM_ENSUREVISIBLE, index, FALSE);
    }
}

void MyTable::RemoveAt(int index)
{
    if (index < 0 || index >= static_cast<int>(m_rows.size())) {
        return;
    }

    m_rows.erase(m_rows.begin() + index);

    if (m_hList != nullptr) {
        m_updating = true;
        SendMessageW(m_hList, LVM_DELETEITEM, index, 0);
        m_updating = false;
    }
}

void MyTable::Clear()
{
    m_rows.clear();

    if (m_hList != nullptr) {
        m_updating = true;
        SendMessageW(m_hList, LVM_DELETEALLITEMS, 0, 0);
        m_updating = false;
    }
}

void MyTable::InsertRow(int index, const Row& row)
{
    LVITEMW item = {};

    item.mask = LVIF_TEXT;
    item.iItem = index;
    item.pszText = const_cast<LPWSTR>(row.name.c_str());

    SendMessageW(m_hList, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item));

    const long values[4] = { row.x1, row.y1, row.x2, row.y2 };

    for (int i = 0; i < 4; ++i) {
        wchar_t text[32];
        swprintf_s(text, L"%ld", values[i]);

        LVITEMW sub = {};

        sub.iSubItem = i + 1;
        sub.pszText = text;

        SendMessageW(
            m_hList, LVM_SETITEMTEXTW,
            index, reinterpret_cast<LPARAM>(&sub)
        );
    }
}

int MyTable::GetSelected() const
{
    if (m_hList == nullptr) {
        return -1;
    }

    return static_cast<int>(
        SendMessageW(m_hList, LVM_GETNEXTITEM, -1, LVNI_SELECTED)
        );
}

INT_PTR CALLBACK MyTable::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    MyTable* table = reinterpret_cast<MyTable*>(GetWindowLongPtrW(hDlg, DWLP_USER));

    if (message == WM_INITDIALOG) {
        table = reinterpret_cast<MyTable*>(lParam);

        SetWindowLongPtrW(
            hDlg,
            DWLP_USER,
            reinterpret_cast<LONG_PTR>(table)
        );
    }

    if (table != nullptr) {
        return table->HandleMessage(hDlg, message, wParam, lParam);
    }

    return FALSE;
}

INT_PTR MyTable::HandleMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        OnInitDialog(hDlg);
        return TRUE;

    case WM_SIZE:
        OnSize(LOWORD(lParam), HIWORD(lParam));
        return TRUE;

    case WM_NOTIFY:
        OnNotify(reinterpret_cast<NMHDR*>(lParam));
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_DELETE) {
            OnDeleteRow();
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hDlg);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        return TRUE;

    case WM_DESTROY:
        m_hDlg = nullptr;
        m_hList = nullptr;
        m_hDelete = nullptr;

        if (m_onSelect) {
            m_onSelect(-1);
        }
        return TRUE;
    }

    return FALSE;
}

void MyTable::OnInitDialog(HWND hDlg)
{
    m_hDlg = hDlg;

    SetWindowTextW(hDlg, L"Таблиця об'єктів");

    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(
        GetWindowLongPtrW(hDlg, GWLP_HINSTANCE)
        );

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    m_hList = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0,
        hDlg,
        nullptr,
        hInstance,
        nullptr
    );

    SendMessageW(
        m_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    const wchar_t* titles[5] = { L"Назва", L"x1", L"y1", L"x2", L"y2" };
    const int widths[5] = { 140, 60, 60, 60, 60 };

    for (int i = 0; i < 5; ++i) {
        LVCOLUMNW column = {};

        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<LPWSTR>(titles[i]);
        column.cx = widths[i];
        column.iSubItem = i;

        SendMessageW(
            m_hList, LVM_INSERTCOLUMNW,
            i, reinterpret_cast<LPARAM>(&column)
        );
    }

    m_hDelete = CreateWindowExW(
        0, L"BUTTON", L"Видалити",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hDlg,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_DELETE)),
        hInstance,
        nullptr
    );

    HWND closeButton = CreateWindowExW(
        0, L"BUTTON", L"Закрити",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        0, 0, 0, 0,
        hDlg,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDCANCEL)),
        hInstance,
        nullptr
    );

    SendMessageW(m_hList, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageW(m_hDelete, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageW(closeButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

    int x = 100;
    int y = 100;

    HWND owner = GetWindow(hDlg, GW_OWNER);

    if (owner != nullptr) {
        RECT ownerRect;
        GetWindowRect(owner, &ownerRect);

        x = ownerRect.right - DIALOG_WIDTH - 30;
        y = ownerRect.top + 100;
    }

    SetWindowPos(hDlg, nullptr, x, y, DIALOG_WIDTH, DIALOG_HEIGHT, SWP_NOZORDER);

    RECT clientRect;
    GetClientRect(hDlg, &clientRect);
    OnSize(clientRect.right, clientRect.bottom);

    for (size_t i = 0; i < m_rows.size(); ++i) {
        InsertRow(static_cast<int>(i), m_rows[i]);
    }
}

void MyTable::OnSize(int width, int height)
{
    if (m_hList == nullptr || m_hDlg == nullptr) {
        return;
    }

    int buttonTop = height - MARGIN - BUTTON_HEIGHT;

    MoveWindow(m_hList, 0, 0, width, buttonTop - MARGIN, TRUE);

    MoveWindow(
        m_hDelete, MARGIN, buttonTop,
        BUTTON_WIDTH, BUTTON_HEIGHT, TRUE
    );

    MoveWindow(
        GetDlgItem(m_hDlg, IDCANCEL), width - MARGIN - BUTTON_WIDTH, buttonTop,
        BUTTON_WIDTH, BUTTON_HEIGHT, TRUE
    );
}

void MyTable::OnNotify(NMHDR* notification)
{
    if (notification == nullptr || notification->hwndFrom != m_hList) {
        return;
    }

    if (notification->code == LVN_ITEMCHANGED) {
        NMLISTVIEW* info = reinterpret_cast<NMLISTVIEW*>(notification);

        if (m_updating || !m_onSelect || !(info->uChanged & LVIF_STATE)) {
            return;
        }

        bool wasSelected = (info->uOldState & LVIS_SELECTED) != 0;
        bool isSelected = (info->uNewState & LVIS_SELECTED) != 0;

        if (wasSelected != isSelected) {
            m_onSelect(GetSelected());
        }
    }

    if (notification->code == LVN_KEYDOWN) {
        NMLVKEYDOWN* key = reinterpret_cast<NMLVKEYDOWN*>(notification);

        if (key->wVKey == VK_DELETE) {
            OnDeleteRow();
        }
    }
}

void MyTable::OnDeleteRow()
{
    int index = GetSelected();

    if (index < 0) {
        return;
    }

    RemoveAt(index);

    if (m_onDelete) {
        m_onDelete(index);
    }
}