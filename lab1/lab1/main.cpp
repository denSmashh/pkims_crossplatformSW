#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tchar.h>
#include <iomanip>

const wchar_t windowClass[] = L"win32app";
const wchar_t windowTitle[] = L"Microwind (Beta Version)";

enum POPUP_MENU_ID
{
    OPEN_ID,
    SAVE_AS_ID,
    EXIT_ID,
    METAL_ID,
    POLY_ID
};

COLORREF POLY_COLOR      RGB(255, 0,  0);
COLORREF METAL_COLOR     RGB(0,   50, 255);
COLORREF INTERSECT_COLOR RGB(250, 20, 147);

class RectLayout
{
public:
    std::string shape;
    std::string substance;
    RECT rect;
    COLORREF color;

    RectLayout();
    ~RectLayout();
};

RectLayout::RectLayout() {}
RectLayout::~RectLayout() {}

std::vector<RectLayout> layout;

bool is_drawing = false;
COLORREF current_color = METAL_COLOR;
POINT drawing_start_point, drawing_end_point;

float scale = 1.0;
int layout_max_x = 0;
int layout_max_y = 0;

void open_file(HWND hWnd, std::vector<RectLayout>& layout)
{
    wchar_t open_szFile[MAX_PATH] = L"";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = open_szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"txt";

    if (GetOpenFileName(&ofn))
    {

        std::fstream in_file(ofn.lpstrFile);
        if (in_file.is_open())
        {
            layout_max_x = 0;
            layout_max_y = 0;
            layout.clear();

            std::string line;
            RectLayout line_rect;
            while (std::getline(in_file, line))
            {
                std::istringstream proc_line(line);
                proc_line >> line_rect.shape;
                proc_line >> line_rect.rect.left;
                proc_line >> line_rect.rect.top;
                proc_line >> line_rect.rect.right;
                proc_line >> line_rect.rect.bottom;
                proc_line >> line_rect.substance;
                line_rect.rect.right = line_rect.rect.left + line_rect.rect.right;
                line_rect.rect.bottom = line_rect.rect.top + line_rect.rect.bottom;
                if (line_rect.substance == "METAL") line_rect.color = METAL_COLOR;
                else if (line_rect.substance == "POLY") line_rect.color = POLY_COLOR;
                layout.push_back(line_rect);

                layout_max_x = max(layout_max_x, line_rect.rect.right);
                layout_max_y = max(layout_max_y, line_rect.rect.bottom);
            }
        }
        else
        {
            MessageBox(NULL, ofn.lpstrFile, L"FILE NOT OPEN", MB_OK);
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        }

        in_file.close();
    }

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    float scaleX = static_cast<float>(clientWidth) / layout_max_x;
    float scaleY = static_cast<float>(clientHeight) / layout_max_y;
    scale = min(scaleX, scaleY);
    InvalidateRect(hWnd, nullptr, 1);
}

void save_file(HWND hWnd, std::vector<RectLayout>& layout)
{
    TCHAR save_as_szFile[MAX_PATH] = _T("");
    OPENFILENAME save_ofn;
    ZeroMemory(&save_ofn, sizeof(save_ofn));
    save_ofn.lStructSize = sizeof(save_ofn);
    save_ofn.hwndOwner = hWnd;
    save_ofn.lpstrFile = save_as_szFile;
    save_ofn.nMaxFile = sizeof(save_as_szFile) / sizeof(TCHAR);
    save_ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
    save_ofn.nFilterIndex = 1;
    save_ofn.lpstrFileTitle = NULL;
    save_ofn.nMaxFileTitle = 0;
    save_ofn.lpstrInitialDir = NULL;
    save_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
    if (GetSaveFileName(&save_ofn))
    {
        std::ofstream out_file(save_ofn.lpstrFile);
        if (out_file.is_open())
        {
            out_file << std::left;
            for (size_t i = 0; i < layout.size(); i++)
            {
                out_file << std::setw(5) << layout[i].shape;
                out_file << std::setw(5) << layout[i].rect.left;
                out_file << std::setw(5) << layout[i].rect.top;
                out_file << std::setw(5) << layout[i].rect.right - layout[i].rect.left;
                out_file << std::setw(5) << layout[i].rect.bottom - layout[i].rect.top;
                out_file << std::setw(5) << layout[i].substance;
                if (i != layout.size() - 1) out_file << "\n";
            }
        }
        else
        {
            MessageBox(NULL, save_ofn.lpstrFile, L"OUTPUT FILE NOT OPEN", MB_OK);
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        }

        out_file.close();
    }
}

void draw_layout(HDC hdc, HWND hWnd, std::vector<RectLayout>& layout)
{
    HPEN pen, old_pen;
    HBRUSH brush, old_brush;

    for (size_t i = 0; i < layout.size(); i++) 
    {
        int rect_left   = layout[i].rect.left   * scale;
        int rect_top    = layout[i].rect.top    * scale;
        int rect_right  = layout[i].rect.right  * scale;
        int rect_bottom = layout[i].rect.bottom * scale;
        RECT scaledRect = {rect_left, rect_top, rect_right, rect_bottom};
        HBRUSH hBrush = CreateSolidBrush(layout[i].color);

        FillRect(hdc, &scaledRect, hBrush);
        DeleteObject(hBrush);

        for (size_t j = 0; j < layout.size(); j++)
        {
            if (layout[i].color != layout[j].color)
            {
                if (&layout[i] != &layout[j])
                {
                    int rect_left   = layout[j].rect.left   * scale;
                    int rect_top    = layout[j].rect.top    * scale;
                    int rect_right  = layout[j].rect.right  * scale;
                    int rect_bottom = layout[j].rect.bottom * scale;
                    RECT scaledOtherRect = { rect_left, rect_top, rect_right, rect_bottom };
                    RECT intersectRect;

                    if (IntersectRect(&intersectRect, &scaledRect, &scaledOtherRect))
                    {
                        HBRUSH hIntersectBrush = CreateSolidBrush(INTERSECT_COLOR);
                        HDC memDC = CreateCompatibleDC(hdc);
                        SelectObject(memDC, hIntersectBrush);

                        int prevMode = SetROP2(hdc, R2_XORPEN);
                        FillRect(hdc, &intersectRect, hIntersectBrush);
                        SetROP2(hdc, prevMode);

                        DeleteObject(hIntersectBrush);
                        DeleteDC(memDC);
                    }
                }
            }
        }
    }
}


long __stdcall WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    PAINTSTRUCT ps;
    HDC hdc;

    RECT r;
    GetClientRect(hWnd, &r);

    switch (message) {
    case WM_CREATE:
    {
        HMENU hMenubar = CreateMenu();

        HMENU hPopupFileMenu = CreatePopupMenu();
        AppendMenu(hPopupFileMenu, MF_STRING, OPEN_ID, L"Open");
        AppendMenu(hPopupFileMenu, MF_STRING, SAVE_AS_ID, L"Save as");
        AppendMenu(hPopupFileMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hPopupFileMenu, MF_STRING, EXIT_ID, L"Exit");

        HMENU hPopupDrawMenu = CreatePopupMenu();
        AppendMenu(hPopupDrawMenu, MF_STRING, METAL_ID, L"Metal");
        AppendMenu(hPopupDrawMenu, MF_STRING, POLY_ID, L"Poly");

        AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hPopupFileMenu, L"&File");
        AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hPopupDrawMenu, L"&Draw");

        SetMenu(hWnd, hMenubar);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        is_drawing = true;
        drawing_start_point.x = LOWORD(lParam);
        drawing_start_point.y = HIWORD(lParam);
        break;
    }

    case WM_LBUTTONUP:
    {
        if (is_drawing)
        {
            is_drawing = false;
            RECT draw_rect = { drawing_start_point.x, drawing_start_point.y, LOWORD(lParam), HIWORD(lParam) };
            if (!(draw_rect.right == 0 || draw_rect.bottom == 0))
            {
                RectLayout drawing_rect;

                int end_x = static_cast<int>(LOWORD(lParam) / scale);
                int end_y = static_cast<int>(HIWORD(lParam) / scale);
                int start_x = static_cast<int>(drawing_start_point.x / scale);
                int start_y = static_cast<int>(drawing_start_point.y / scale);

                RECT rect = { min(start_x, end_x), min(start_y, end_y), max(start_x, end_x), max(start_y, end_y) };

                layout_max_x = max(layout_max_x, drawing_rect.rect.right);
                layout_max_y = max(layout_max_y, drawing_rect.rect.bottom);

                drawing_rect.rect = rect;
                drawing_rect.shape = "REC";
                if (current_color == METAL_COLOR)
                {
                    drawing_rect.color = current_color;
                    drawing_rect.substance = "METAL";
                }
                else if (current_color == POLY_COLOR)
                {
                    drawing_rect.color = current_color;
                    drawing_rect.substance = "POLY";
                }
                layout.push_back(drawing_rect);
            }
            InvalidateRect(hWnd, nullptr, 1);
        }
        break;
    }

    case WM_COMMAND:
    {
        switch ((LOWORD(wParam)))
        {
        case OPEN_ID:
            open_file(hWnd, layout);
            InvalidateRect(hWnd, nullptr, 1);
            break;

        case SAVE_AS_ID:
            save_file(hWnd, layout);
            break;

        case EXIT_ID:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
            break;

        case METAL_ID:
            current_color = METAL_COLOR;
            break;

        case POLY_ID:
            current_color = POLY_COLOR;
            break;
        }
        break;
    }

    case WM_SIZE:
    {
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int currentWidth = clientRect.right - clientRect.left;
        int currentHeight = clientRect.bottom - clientRect.top;
        float scaleX = static_cast<float>(currentWidth) / layout_max_x;
        float scaleY = static_cast<float>(currentHeight) / layout_max_y;
        scale = min(scaleX, scaleY);
        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);
        draw_layout(hdc, hWnd, layout);
        EndPaint(hWnd, &ps);
        break;
    }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = windowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, L"Can’t register window class!", L"Win32 API Test", NULL);
        return 1;
    }

    HWND hWnd = CreateWindow(windowClass, windowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        MessageBox(NULL, L"Can’t create window!", L"Win32 API Test", NULL);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
