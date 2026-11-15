#include "TableView.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace
{
    const int CELL_PADDING = 8;
    const int ARROW_WIDTH = 24;
    const int MIN_COL_WIDTH = 40;
    const int LINE_STEP = 20;

    // текст файлу (UTF-8)
    std::wstring ToWide(const std::string& bytes)
    {
        size_t start = 0;

        if (bytes.size() >= 3 &&
            (unsigned char)bytes[0] == 0xEF &&
            (unsigned char)bytes[1] == 0xBB &&
            (unsigned char)bytes[2] == 0xBF) {
            start = 3;
        }

        int len = (int)(bytes.size() - start);

        if (len == 0) {
            return L"";
        }

        const char* src = bytes.data() + start;
        int count = MultiByteToWideChar(CP_UTF8, 0, src, len, NULL, 0);

        std::wstring result(count, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, src, len, &result[0], count);

        return result;
    }

    // wstring -> UTF-8
    std::string ToUtf8(const std::wstring& text)
    {
        if (text.empty()) {
            return "";
        }

        int len = (int)text.size();
        int count = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), len, NULL, 0, NULL, NULL);

        std::string result(count, '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), len, &result[0], count, NULL, NULL);

        return result;
    }

    std::vector<std::wstring> SplitLine(const std::wstring& line)
    {
        std::vector<std::wstring> cells;
        size_t pos = 0;

        while (true) {
            size_t tab = line.find(L'\t', pos);

            if (tab == std::wstring::npos) {
                cells.push_back(line.substr(pos));
                break;
            }

            cells.push_back(line.substr(pos, tab - pos));
            pos = tab + 1;
        }

        return cells;
    }

    std::wstring JoinLine(const std::vector<std::wstring>& cells)
    {
        std::wstring line;

        for (size_t i = 0; i < cells.size(); ++i) {
            if (i > 0) {
                line += L'\t';
            }

            line += cells[i];
        }

        line += L"\r\n";

        return line;
    }


    bool HasNumberColumn(const std::vector<std::wstring>& headers)
    {
        if (headers.empty()) {
            return false;
        }

        const std::wstring& first = headers[0];

        return first == L"\u2116" || first == L"#" || first == L"No";
    }
}

TableViewClass::TableViewClass()
    : sortColumn(-1),
    sortState(0),
    scrollX(0),
    scrollY(0),
    rowHeight(20),
    headerHeight(24)
{
}

bool TableViewClass::IsEmpty() const
{
    return headers.empty();
}

int TableViewClass::ContentWidth() const
{
    int width = 0;

    for (size_t i = 0; i < colWidths.size(); ++i) {
        width += colWidths[i];
    }

    return width;
}

int TableViewClass::ContentHeight() const
{
    return (int)rows.size() * rowHeight;
}



// ---------- файли ----------

bool TableViewClass::Load(HWND hwnd, const wchar_t* path)
{
    FILE* file = nullptr;

    if (_wfopen_s(&file, path, L"rb") != 0 || file == nullptr) {
        return false;
    }

    std::string bytes;
    char buffer[4096];
    size_t count;

    while ((count = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        bytes.append(buffer, count);
    }

    fclose(file);

    std::wstring text = ToWide(bytes);

    Row newHeaders;
    std::vector<Row> newRows;
    bool first = true;
    size_t pos = 0;

    while (pos < text.size()) {
        size_t end = text.find(L'\n', pos);

        if (end == std::wstring::npos) {
            end = text.size();
        }

        std::wstring line = text.substr(pos, end - pos);
        pos = end + 1;

        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        if (first) {
            newHeaders = SplitLine(line);
            first = false;
        } else {
            newRows.push_back(SplitLine(line));
        }
    }

    if (first) {
        return false;
    }

    // кількість стовпців у всіх рядках
    size_t columns = newHeaders.size();

    for (size_t i = 0; i < newRows.size(); ++i) {
        columns = (std::max)(columns, newRows[i].size());
    }

    newHeaders.resize(columns);

    for (size_t i = 0; i < newRows.size(); ++i) {
        newRows[i].resize(columns);
    }

    // якщо стовпчика з номерами немає, додаємо його самостійно
    if (!HasNumberColumn(newHeaders)) {
        newHeaders.insert(newHeaders.begin(), L"\u2116");

        for (size_t i = 0; i < newRows.size(); ++i) {
            newRows[i].insert(newRows[i].begin(), std::to_wstring(i + 1));
        }
    }

    headers = newHeaders;
    rows = newRows;


    sortColumn = -1;
    sortState = 0;
    scrollX = 0;
    scrollY = 0;

    ApplySort();
    Measure(hwnd);
    UpdateScrollBars(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);

    return true;
}

// запис у початковому порядку рядків, незалежно від сортування на екрані
bool TableViewClass::Save(const wchar_t* path) const
{
    if (headers.empty()) {
        return false;
    }

    std::wstring text = JoinLine(headers);

    for (size_t i = 0; i < rows.size(); ++i) {
        text += JoinLine(rows[i]);
    }

    std::string bytes = ToUtf8(text);

    FILE* file = nullptr;

    if (_wfopen_s(&file, path, L"wb") != 0 || file == nullptr) {
        return false;
    }

    size_t written = fwrite(bytes.data(), 1, bytes.size(), file);
    bool success = (written == bytes.size());

    if (fclose(file) != 0) {
        success = false;
    }

    return success;
}



// ---------- розміри та сортування ----------

void TableViewClass::Measure(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    HFONT oldFont = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);

    rowHeight = tm.tmHeight + 6;
    headerHeight = rowHeight + 4;

    colWidths.assign(headers.size(), MIN_COL_WIDTH);

    for (size_t c = 0; c < headers.size(); ++c) {
        SIZE size;

        GetTextExtentPoint32W(
            hdc, headers[c].c_str(), (int)headers[c].size(), &size
        );

        int width = size.cx + 2 * CELL_PADDING + ARROW_WIDTH;

        for (size_t r = 0; r < rows.size(); ++r) {
            GetTextExtentPoint32W(
                hdc, rows[r][c].c_str(), (int)rows[r][c].size(), &size
            );

            width = (std::max)(width, (int)size.cx + 2 * CELL_PADDING);
        }

        colWidths[c] = (std::max)(width, MIN_COL_WIDTH);
    }

    SelectObject(hdc, oldFont);
    ReleaseDC(hwnd, hdc);
}

void TableViewClass::ApplySort()
{
    order.resize(rows.size());

    for (size_t i = 0; i < order.size(); ++i) {
        order[i] = (int)i;
    }

    if (sortState == 0 || sortColumn < 0) {
        return;
    }

    int column = sortColumn;
    bool ascending = (sortState == 1);

    std::stable_sort(
        order.begin(), order.end(),
        [&](int a, int b) {
            const std::wstring& sa = rows[a][column];
            const std::wstring& sb = rows[b][column];

            int result;

            if (column == 0) {
                double na = wcstod(sa.c_str(), nullptr);
                double nb = wcstod(sb.c_str(), nullptr);

                result = (na < nb) ? -1 : (na > nb) ? 1 : 0;
            } else {
                result = CompareStringW(
                    LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                    sa.c_str(), -1, sb.c_str(), -1
                ) - 2;
            }

            return ascending ? (result < 0) : (result > 0);
        }
    );
}



void TableViewClass::UpdateScrollBars(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    int viewWidth = rc.right;
    int viewHeight = (std::max)(0, (int)rc.bottom - headerHeight);

    int maxX = (std::max)(0, ContentWidth() - viewWidth);
    int maxY = (std::max)(0, ContentHeight() - viewHeight);

    scrollX = (std::min)((std::max)(scrollX, 0), maxX);
    scrollY = (std::min)((std::max)(scrollY, 0), maxY);

    SCROLLINFO si = {};

    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

    si.nMin = 0;
    si.nMax = (std::max)(0, ContentWidth() - 1);
    si.nPage = viewWidth;
    si.nPos = scrollX;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    si.nMax = (std::max)(0, ContentHeight() - 1);
    si.nPage = viewHeight;
    si.nPos = scrollY;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void TableViewClass::ScrollTo(HWND hwnd, int x, int y)
{
    scrollX = x;
    scrollY = y;

    UpdateScrollBars(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
}

void TableViewClass::Scroll(HWND hwnd, int bar, WPARAM wParam)
{
    SCROLLINFO si = {};

    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd, bar, &si);

    int pos = si.nPos;
    int line = (bar == SB_VERT) ? rowHeight : LINE_STEP;

    switch (LOWORD(wParam)) {
    case SB_LINEUP:
        pos -= line;
        break;

    case SB_LINEDOWN:
        pos += line;
        break;

    case SB_PAGEUP:
        pos -= (int)si.nPage;
        break;

    case SB_PAGEDOWN:
        pos += (int)si.nPage;
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        pos = si.nTrackPos;
        break;

    case SB_TOP:
        pos = 0;
        break;

    case SB_BOTTOM:
        pos = si.nMax;
        break;

    default:
        return;
    }

    if (bar == SB_VERT) {
        ScrollTo(hwnd, scrollX, pos);
    } else {
        ScrollTo(hwnd, pos, scrollY);
    }
}

void TableViewClass::OnHScroll(HWND hwnd, WPARAM wParam)
{
    Scroll(hwnd, SB_HORZ, wParam);
}

void TableViewClass::OnVScroll(HWND hwnd, WPARAM wParam)
{
    Scroll(hwnd, SB_VERT, wParam);
}

void TableViewClass::OnMouseWheel(HWND hwnd, WPARAM wParam)
{
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    ScrollTo(hwnd, scrollX, scrollY - delta / WHEEL_DELTA * 3 * rowHeight);
}

void TableViewClass::OnSize(HWND hwnd)
{
    UpdateScrollBars(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
}



// ---------- сортування мишею ----------

void TableViewClass::OnRButtonDown(HWND hwnd, LPARAM lParam)
{
    int x = (short)LOWORD(lParam);
    int y = (short)HIWORD(lParam);

    if (headers.empty() || y < 0 || y >= headerHeight) {
        return;
    }

    x += scrollX;

    int left = 0;
    int column = -1;

    for (size_t c = 0; c < colWidths.size(); ++c) {
        if (x >= left && x < left + colWidths[c]) {
            column = (int)c;
            break;
        }

        left += colWidths[c];
    }

    if (column < 0) {
        return;
    }

    // цикл за зростанням, за зменшенням і без сортування
    if (column != sortColumn) {
        sortColumn = column;
        sortState = 1;
    } else {
        sortState = (sortState + 1) % 3;
    }

    ApplySort();
    InvalidateRect(hwnd, NULL, FALSE);
}



void TableViewClass::Draw(HDC hdc, const RECT& rc) const
{
    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

    HFONT oldFont = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(hdc, TRANSPARENT);

    if (headers.empty()) {
        const wchar_t hint[] = L"Open file: File -> Open...";
        TextOutW(hdc, 10, 10, hint, (int)wcslen(hint));

        SelectObject(hdc, oldFont);
        return;
    }

    HBRUSH altBrush = CreateSolidBrush(RGB(244, 247, 252));
    HBRUSH headerBrush = CreateSolidBrush(RGB(225, 225, 225));
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN darkPen = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));
    HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

    const UINT flags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

    int rowCount = (int)rows.size();
    int firstRow = scrollY / rowHeight;
    int lastRow = (scrollY + rc.bottom - headerHeight) / rowHeight;

    if (lastRow > rowCount - 1) {
        lastRow = rowCount - 1;
    }


    for (int r = firstRow; r <= lastRow; ++r) {
        int y = headerHeight + r * rowHeight - scrollY;

        if (r % 2 == 1) {
            RECT rowRect = { 0, y, rc.right, y + rowHeight };
            FillRect(hdc, &rowRect, altBrush);
        }

        const Row& row = rows[order[r]];
        int x = -scrollX;

        for (size_t c = 0; c < colWidths.size(); ++c) {
            int width = colWidths[c];

            if (x + width > 0 && x < rc.right) {
                RECT cell = { x + CELL_PADDING, y, x + width - CELL_PADDING, y + rowHeight };
                DrawTextW(hdc, row[c].c_str(), -1, &cell, flags);
            }

            x += width;
        }

        MoveToEx(hdc, 0, y + rowHeight - 1, NULL);
        LineTo(hdc, rc.right, y + rowHeight - 1);
    }


    int x = -scrollX;

    for (size_t c = 0; c < colWidths.size(); ++c) {
        x += colWidths[c];

        MoveToEx(hdc, x - 1, headerHeight, NULL);
        LineTo(hdc, x - 1, rc.bottom);
    }


    RECT headerRect = { 0, 0, rc.right, headerHeight };
    FillRect(hdc, &headerRect, headerBrush);

    SelectObject(hdc, darkPen);

    x = -scrollX;

    for (size_t c = 0; c < colWidths.size(); ++c) {
        int width = colWidths[c];

        if (x + width > 0 && x < rc.right) {
            RECT cell = { x + CELL_PADDING, 0, x + width - ARROW_WIDTH, headerHeight };
            DrawTextW(hdc, headers[c].c_str(), -1, &cell, flags);


            if ((int)c == sortColumn && sortState != 0) {
                int cx = x + width - 14;
                int cy = headerHeight / 2;

                POINT up[3] = { { cx - 5, cy + 3 }, { cx + 5, cy + 3 }, { cx, cy - 4 } };
                POINT down[3] = { { cx - 5, cy - 3 }, { cx + 5, cy - 3 }, { cx, cy + 4 } };

                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(BLACK_BRUSH));
                Polygon(hdc, (sortState == 1) ? up : down, 3);
                SelectObject(hdc, oldBrush);
            }
        }

        x += width;

        MoveToEx(hdc, x - 1, 0, NULL);
        LineTo(hdc, x - 1, headerHeight);
    }

    MoveToEx(hdc, 0, headerHeight - 1, NULL);
    LineTo(hdc, rc.right, headerHeight - 1);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldFont);

    DeleteObject(altBrush);
    DeleteObject(headerBrush);
    DeleteObject(gridPen);
    DeleteObject(darkPen);
}

void TableViewClass::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    if (rc.right > 0 && rc.bottom > 0) {
        HDC memoryDC = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);

        if (memoryDC != NULL && bitmap != NULL) {
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);

            Draw(memoryDC, rc);
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memoryDC, 0, 0, SRCCOPY);

            SelectObject(memoryDC, oldBitmap);
        } else {
            Draw(hdc, rc);
        }

        if (bitmap != NULL) {
            DeleteObject(bitmap);
        }

        if (memoryDC != NULL) {
            DeleteDC(memoryDC);
        }
    }

    EndPaint(hwnd, &ps);
}