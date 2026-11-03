#include "framework.h"
#include "Lab5.h"
#include "Resource.h"
#include "lab5_app.h"

#include "my_editor.h"
#include "shape.h"
#include "shape_factory.h"
#include "point_shape.h"
#include "line_shape.h"
#include "rect_shape.h"
#include "ellipse_shape.h"
#include "line_oo_shape.h"
#include "cube_shape.h"

#pragma comment(lib, "Comctl32.lib")

namespace
{
    const wchar_t WINDOW_CLASS_NAME[] = L"Lab5WindowClass";

    const int TOOLBAR_HEIGHT = 36;
    const int TOOL_ICON_SIZE = 24;
    const int TOOL_COUNT = 6;

    const int TOOL_COMMANDS[TOOL_COUNT] = {
        IDM_POINT, IDM_LINE, IDM_RECT,
        IDM_ELLIPSE, IDM_LINEOO, IDM_CUBE
    };
}


MyEditor& ED = MyEditor::getInstance();

Lab5App::Lab5App(HINSTANCE hInstance)
    : m_hInstance(hInstance),
    m_hWnd(nullptr),
    m_toolbar(nullptr),
    m_imageList(nullptr),
    m_objectsMenu(nullptr),
    m_windowMenu(nullptr),
    m_currentTool(IDM_POINT)
{
}

Lab5App::~Lab5App()
{
    if (m_imageList != nullptr) {
        ImageList_Destroy(m_imageList);
    }
}

int Lab5App::Run(int nCmdShow)
{
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = m_hInstance;
    windowClass.hIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_LAB5));
    windowClass.hIconSm = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_SMALL));
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB5);
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

    HACCEL accelerators = LoadAcceleratorsW(m_hInstance, MAKEINTRESOURCEW(IDC_LAB5));

    MSG message = {};
    BOOL result;

    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (m_table.ProcessMessage(&message)) {
            continue;
        }

        if (accelerators == nullptr || !TranslateAcceleratorW(window, accelerators, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    return static_cast<int>(message.wParam);
}

LRESULT CALLBACK Lab5App::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Lab5App* app = reinterpret_cast<Lab5App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lParam);

        app = static_cast<Lab5App*>(create->lpCreateParams);

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

LRESULT Lab5App::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        ED.OnLBdown(hwnd);
        return 0;

    case WM_LBUTTONUP:
        ED.OnLBup(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        ED.OnMouseMove(hwnd);
        return 0;

    case WM_PAINT:
        ED.OnPaint(hwnd);
        return 0;

    case WM_DESTROY:
        m_table.Close();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

bool Lab5App::OnCreate()
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
        TOOL_COUNT,
        0
    );

    if (m_imageList == nullptr) {
        return false;
    }

    for (int i = 0; i < TOOL_COUNT; ++i) {
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

    TBBUTTON buttons[TOOL_COUNT] = {};

    for (int i = 0; i < TOOL_COUNT; ++i) {
        buttons[i].iBitmap = i;
        buttons[i].idCommand = TOOL_COMMANDS[i];
        buttons[i].fsState = TBSTATE_ENABLED;
        buttons[i].fsStyle = BTNS_BUTTON;
        buttons[i].iString = -1;
    }

    if (!SendMessageW(
        m_toolbar,
        TB_ADDBUTTONSW,
        TOOL_COUNT,
        reinterpret_cast<LPARAM>(buttons)
    )) {
        return false;
    }

    SendMessageW(m_toolbar, TB_AUTOSIZE, 0, 0);

    m_objectsMenu = GetSubMenu(GetMenu(m_hWnd), 1);
    m_windowMenu = GetSubMenu(GetMenu(m_hWnd), 2);

    RegisterShapes();

    ED.SetShapeAddedHandler([this](const Shape& shape) {
        OnShapeAdded(shape);
        });

    m_table.SetSelectHandler([this](int index) {
        OnTableSelect(index);
        });

    m_table.SetDeleteHandler([this](int index) {
        OnTableDelete(index);
        });

    RECT clientRect;
    GetClientRect(m_hWnd, &clientRect);
    OnSize(clientRect.right);

    ED.Start(new PointShape);

    return true;
}

void Lab5App::OnSize(int width)
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

LRESULT Lab5App::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int commandId = LOWORD(wParam);

    switch (commandId) {
    case IDM_POINT:
        m_currentTool = IDM_POINT;
        ED.Start(new PointShape);
        return 0;

    case IDM_LINE:
        m_currentTool = IDM_LINE;
        ED.Start(new LineShape);
        return 0;

    case IDM_RECT:
        m_currentTool = IDM_RECT;
        ED.Start(new RectShape);
        return 0;

    case IDM_ELLIPSE:
        m_currentTool = IDM_ELLIPSE;
        ED.Start(new EllipseShape);
        return 0;

    case IDM_LINEOO:
        m_currentTool = IDM_LINEOO;
        ED.Start(new LineOOShape);
        return 0;

    case IDM_CUBE:
        m_currentTool = IDM_CUBE;
        ED.Start(new CubeShape);
        return 0;

    case IDM_TABLE:
        m_table.Toggle(m_hInstance, m_hWnd);
        return 0;

    case IDM_LOAD:
        OnLoad();
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

LRESULT Lab5App::OnNotify(NMHDR* notification)
{
    if (notification == nullptr) {
        return 0;
    }

    if (notification->code == TTN_GETDISPINFOW) {
        NMTTDISPINFOW* tip = reinterpret_cast<NMTTDISPINFOW*>(notification);

        const wchar_t* text = GetToolTip(static_cast<int>(notification->idFrom));

        if (text != nullptr) {
            tip->lpszText = const_cast<LPWSTR>(text);
        }
    }

    return 0;
}

void Lab5App::OnInitMenuPopup(HMENU menu)
{
    if (menu == m_objectsMenu) {
        CheckMenuRadioItem(
            menu,
            IDM_POINT,
            IDM_CUBE,
            m_currentTool,
            MF_BYCOMMAND
        );
    }

    if (menu == m_windowMenu) {
        CheckMenuItem(
            menu,
            IDM_TABLE,
            MF_BYCOMMAND | (m_table.IsOpen() ? MF_CHECKED : MF_UNCHECKED)
        );
    }
}

void Lab5App::RegisterShapes()
{
    ShapeFactory& factory = ShapeFactory::getInstance();

    factory.Register(new PointShape);
    factory.Register(new LineShape);
    factory.Register(new RectShape);
    factory.Register(new EllipseShape);
    factory.Register(new LineOOShape);
    factory.Register(new CubeShape);
}

void Lab5App::OnShapeAdded(const Shape& shape)
{
    long x1, y1, x2, y2;
    shape.Get(x1, y1, x2, y2);

    m_table.Add(shape.Title(), x1, y1, x2, y2);
}

void Lab5App::OnTableSelect(int index)
{
    ED.Select(index);
    InvalidateRect(m_hWnd, NULL, FALSE);
}

void Lab5App::OnTableDelete(int index)
{
    ED.Remove(index);
    InvalidateRect(m_hWnd, NULL, FALSE);
}

void Lab5App::OnLoad()
{
    if (!ED.Load()) {
        MessageBoxW(
            m_hWnd,
            L"Не вдалося відкрити файл shapes.txt",
            L"Завантаження",
            MB_OK | MB_ICONWARNING
        );

        return;
    }

    RefreshTable();
    InvalidateRect(m_hWnd, NULL, FALSE);
}

void Lab5App::RefreshTable()
{
    m_table.Clear();

    for (int i = 0; i < ED.GetCount(); ++i) {
        OnShapeAdded(*ED.GetShape(i));
    }
}

const wchar_t* Lab5App::GetToolTip(int commandId) const
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

    case IDM_LINEOO:
        return L"Draw a line with circles";

    case IDM_CUBE:
        return L"Draw a cube frame";
    }

    return nullptr;
}

HBITMAP Lab5App::CreateToolbarImage(int index)
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

    HPEN pen = CreatePen(PS_SOLID, (index == 5) ? 1 : 2, RGB(0, 0, 0));
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

    case 4:
        MoveToEx(memoryDC, 6, 18, nullptr);
        LineTo(memoryDC, 18, 6);
        Ellipse(memoryDC, 2, 14, 10, 22);
        Ellipse(memoryDC, 14, 2, 22, 10);
        break;

    case 5:
        Rectangle(memoryDC, 3, 9, 15, 21);
        Rectangle(memoryDC, 9, 3, 21, 15);
        MoveToEx(memoryDC, 3, 9, nullptr);
        LineTo(memoryDC, 9, 3);
        MoveToEx(memoryDC, 15, 9, nullptr);
        LineTo(memoryDC, 21, 3);
        MoveToEx(memoryDC, 3, 21, nullptr);
        LineTo(memoryDC, 9, 15);
        MoveToEx(memoryDC, 15, 21, nullptr);
        LineTo(memoryDC, 21, 15);
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

INT_PTR CALLBACK Lab5App::AboutDialogProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam
)
{
    Lab5App* app = reinterpret_cast<Lab5App*>(GetWindowLongPtrW(hwnd, DWLP_USER));

    if (message == WM_INITDIALOG) {
        app = reinterpret_cast<Lab5App*>(lParam);

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

INT_PTR Lab5App::OnAboutDialog(
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