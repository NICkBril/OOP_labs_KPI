#include "framework.h"
#include "Lab3.h"
#include "Resource.h"
#include "lab3_app.h"

#include <cwchar>

#pragma comment(lib, "Comctl32.lib")

namespace
{
    const wchar_t WINDOW_CLASS_NAME[] = L"Lab3WindowClass";

    const int TOOLBAR_HEIGHT = 36;
    const int TOOL_ICON_SIZE = 24;
}

Lab3App::Lab3App(HINSTANCE hInstance)
    : m_hInstance(hInstance),
    m_hWnd(nullptr),
    m_toolbar(nullptr),
    m_imageList(nullptr),
    m_objectsMenu(nullptr),
    m_currentTool(IDM_POINT)
{
}

Lab3App::~Lab3App()
{
    if (m_imageList != nullptr) {
        ImageList_Destroy(m_imageList);
    }
}

int Lab3App::Run(int nCmdShow)
{
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = m_hInstance;
    windowClass.hIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_LAB3));
    windowClass.hIconSm = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB3);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassExW(&windowClass)) {
        return 0;
    }

    HWND window = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        L"Graphic Editor",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1000,
        700,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (window == nullptr) {
        return 0;
    }

    ShowWindow(window, nCmdShow);
    UpdateWindow(window);

    HACCEL accelerators = LoadAcceleratorsW(m_hInstance, MAKEINTRESOURCEW(IDC_LAB3));

    MSG message = {};
    BOOL result;

    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (accelerators == nullptr || !TranslateAcceleratorW(window, accelerators, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    return static_cast<int>(message.wParam);
}

LRESULT CALLBACK Lab3App::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Lab3App* app = reinterpret_cast<Lab3App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lParam);

        app = static_cast<Lab3App*>(create->lpCreateParams);

        if (app == nullptr) {
            return FALSE;
        }

        app->m_hWnd = hwnd;

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(app)
        );
    }

    if (app != nullptr) {
        return app->HandleMessage(hwnd, message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT Lab3App::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        return OnCreate() ? 0 : -1;

    case WM_SIZE:
        OnSize(LOWORD(lParam));
        return 0;

    case WM_COMMAND:
        return OnCommand(wParam, lParam);

    case WM_NOTIFY:
        return OnNotify(reinterpret_cast<NMHDR*>(lParam));

    case WM_INITMENUPOPUP:
        OnInitMenuPopup(reinterpret_cast<HMENU>(wParam));
        return 0;

    case WM_LBUTTONDOWN:
        m_editor.OnLBdown(hwnd);
        return 0;

    case WM_LBUTTONUP:
        m_editor.OnLBup(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        m_editor.OnMouseMove(hwnd);
        return 0;

    case WM_PAINT:
        m_editor.OnPaint(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

bool Lab3App::OnCreate()
{
    m_toolbar = CreateWindowExW(
        0,
        TOOLBARCLASSNAMEW,
        nullptr,
        WS_CHILD | WS_VISIBLE |
        TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
        CCS_NOPARENTALIGN | CCS_NORESIZE,
        0,
        0,
        0,
        TOOLBAR_HEIGHT,
        m_hWnd,
        reinterpret_cast<HMENU>(
            static_cast<INT_PTR>(IDC_TOOLBAR)
            ),
        m_hInstance,
        nullptr
    );

    if (m_toolbar == nullptr) {
        return false;
    }

    SendMessageW(
        m_toolbar, TB_BUTTONSTRUCTSIZE,
        sizeof(TBBUTTON), 0
    );

    SendMessageW(
        m_toolbar, TB_SETBITMAPSIZE,
        0, MAKELONG(TOOL_ICON_SIZE, TOOL_ICON_SIZE)
    );

    SendMessageW(
        m_toolbar, TB_SETBUTTONSIZE,
        0, MAKELONG(32, 32)
    );

    m_imageList = ImageList_Create(
        TOOL_ICON_SIZE,
        TOOL_ICON_SIZE,
        ILC_COLOR32 | ILC_MASK,
        4,
        0
    );

    if (m_imageList == nullptr) {
        return false;
    }

    for (int i = 0; i < 4; ++i) {
        HBITMAP bitmap = CreateToolbarImage(i);

        if (bitmap == nullptr) {
            return false;
        }

        int imageIndex = ImageList_AddMasked(
            m_imageList, bitmap, RGB(255, 255, 255)
        );

        DeleteObject(bitmap);

        if (imageIndex == -1) {
            return false;
        }
    }

    SendMessageW(
        m_toolbar, TB_SETIMAGELIST,
        0, reinterpret_cast<LPARAM>(m_imageList)
    );

    const int commands[] = {
        IDM_POINT,
        IDM_LINE,
        IDM_RECT,
        IDM_ELLIPSE
    };

    TBBUTTON buttons[4] = {};

    for (int i = 0; i < 4; ++i) {
        buttons[i].iBitmap = i;
        buttons[i].idCommand = commands[i];
        buttons[i].fsState = TBSTATE_ENABLED;
        buttons[i].fsStyle = BTNS_BUTTON;
        buttons[i].iString = -1;
    }

    if (!SendMessageW(
        m_toolbar,
        TB_ADDBUTTONSW,
        4,
        reinterpret_cast<LPARAM>(buttons)
    )) {
        return false;
    }

    SendMessageW(m_toolbar, TB_AUTOSIZE, 0, 0);

    m_objectsMenu = GetSubMenu(GetMenu(m_hWnd), 1);

    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    OnSize(clientRect.right);

    m_editor.StartPointEditor(m_hWnd);

    return true;
}

void Lab3App::OnSize(int width)
{
    if (m_toolbar != nullptr) {
        SetWindowPos(
            m_toolbar,
            nullptr,
            0,
            0,
            width,
            TOOLBAR_HEIGHT,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }
}

LRESULT Lab3App::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int commandId = LOWORD(wParam);

    switch (commandId) {
    case IDM_POINT:
        m_currentTool = IDM_POINT;
        m_editor.StartPointEditor(m_hWnd);
        return 0;

    case IDM_LINE:
        m_currentTool = IDM_LINE;
        m_editor.StartLineEditor(m_hWnd);
        return 0;

    case IDM_RECT:
        m_currentTool = IDM_RECT;
        m_editor.StartRectEditor(m_hWnd);
        return 0;

    case IDM_ELLIPSE:
        m_currentTool = IDM_ELLIPSE;
        m_editor.StartEllipseEditor(m_hWnd);
        return 0;

    case IDM_ABOUT:
        DialogBoxParamW(
            m_hInstance,
            MAKEINTRESOURCEW(IDD_ABOUTBOX),
            m_hWnd,
            AboutDialogProc,
            reinterpret_cast<LPARAM>(this)
        );
        return 0;

    case IDM_EXIT:
        DestroyWindow(m_hWnd);
        return 0;
    }

    return DefWindowProcW(m_hWnd, WM_COMMAND, wParam, lParam);
}

LRESULT Lab3App::OnNotify(NMHDR* notification)
{
    if (notification == nullptr) {
        return 0;
    }

    if (notification->hwndFrom == m_toolbar &&
        notification->code == TBN_GETINFOTIPW) {

        NMTBGETINFOTIPW* tip =
            reinterpret_cast<NMTBGETINFOTIPW*>(notification);

        const wchar_t* text = GetToolTipByIndex(tip->iItem);

        if (text != nullptr &&
            tip->pszText != nullptr &&
            tip->cchTextMax > 0) {

            wcsncpy_s(
                tip->pszText,
                static_cast<size_t>(tip->cchTextMax),
                text,
                _TRUNCATE
            );
        }

        return 0;
    }

    if (notification->code == TTN_GETDISPINFOW) {
        NMTTDISPINFOW* tip = reinterpret_cast<NMTTDISPINFOW*>(notification);

        const wchar_t* text = GetToolTipByCommand(static_cast<int>(notification->idFrom));

        if (text != nullptr) {
            tip->lpszText = const_cast<LPWSTR>(text);
            return 0;
        }
    }

    return 0;
}

void Lab3App::OnInitMenuPopup(HMENU menu)
{
    if (menu == m_objectsMenu) {
        CheckMenuRadioItem(
            menu,
            IDM_POINT,
            IDM_ELLIPSE,
            m_currentTool,
            MF_BYCOMMAND
        );
    }
}


const wchar_t* Lab3App::GetToolTipByIndex(int index) const
{
    switch (index) {
    case 0:
        return L"Draw a point";

    case 1:
        return L"Draw a line";

    case 2:
        return L"Draw a rectangle";

    case 3:
        return L"Draw an ellipse";
    }

    return nullptr;
}

const wchar_t* Lab3App::GetToolTipByCommand(int commandId) const
{
    switch (commandId) {
    case IDM_POINT:
        return L"Draw a point";

    case IDM_LINE:
        return L"Draw a line";

    case IDM_RECT:
        return L"Draw a rectangle";

    case IDM_ELLIPSE:
        return L"Draw an ellipse";
    }

    return nullptr;
}


HBITMAP Lab3App::CreateToolbarImage(int index)
{
    HDC windowDC = GetDC(m_hWnd);

    if (windowDC == nullptr) {
        return nullptr;
    }

    HDC memoryDC = CreateCompatibleDC(windowDC);

    if (memoryDC == nullptr) {
        ReleaseDC(m_hWnd, windowDC);

        return nullptr;
    }

    HBITMAP bitmap = CreateCompatibleBitmap(windowDC, TOOL_ICON_SIZE, TOOL_ICON_SIZE);

    if (bitmap == nullptr) {
        DeleteDC(memoryDC);
        ReleaseDC(m_hWnd, windowDC);

        return nullptr;
    }

    HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);

    RECT rect = { 0, 0, TOOL_ICON_SIZE, TOOL_ICON_SIZE };

    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(memoryDC, &rect, background);
    DeleteObject(background);

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(memoryDC, pen);

    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

    HBRUSH oldBrush = (HBRUSH)SelectObject(memoryDC, nullBrush);

    switch (index) {
    case 0:
    {
        HBRUSH dotBrush = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH previousBrush = (HBRUSH)SelectObject(memoryDC, dotBrush);

        Ellipse(memoryDC, 8, 8, 16, 16);

        SelectObject(memoryDC, previousBrush);
        DeleteObject(dotBrush);
        break;
    }

    case 1:
        MoveToEx(memoryDC, 4, 20, nullptr);
        LineTo(memoryDC, 20, 4);
        break;

    case 2:
        Rectangle(memoryDC, 4, 5, 20, 19);
        break;

    case 3:
        Ellipse(memoryDC, 4, 4, 20, 20);
        break;
    }

    SelectObject(memoryDC, oldBrush);
    SelectObject(memoryDC, oldPen);
    SelectObject(memoryDC, oldBitmap);

    DeleteObject(pen);
    DeleteDC(memoryDC);
    ReleaseDC(m_hWnd, windowDC);

    return bitmap;
}

INT_PTR CALLBACK Lab3App::AboutDialogProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam
)
{
    Lab3App* app = reinterpret_cast<Lab3App*>(GetWindowLongPtrW(hwnd, DWLP_USER));

    if (message == WM_INITDIALOG) {
        app = reinterpret_cast<Lab3App*>(lParam);

        SetWindowLongPtrW(
            hwnd,
            DWLP_USER,
            reinterpret_cast<LONG_PTR>(app)
        );
    }

    if (app != nullptr) {
        return app->OnAboutDialog(hwnd, message, wParam, lParam);
    }

    return FALSE;
}

INT_PTR Lab3App::OnAboutDialog(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM
)
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {

            EndDialog(hwnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}