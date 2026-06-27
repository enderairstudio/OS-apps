#include "core/FindDialog.hpp"

namespace PaperPad {

struct FindDialogData {
    std::wstring query;
    bool confirmed = false;
    HWND hEdit = NULL;
};

static LRESULT CALLBACK FindDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    FindDialogData* data = reinterpret_cast<FindDialogData*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    switch (message) {
        case WM_CREATE: {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            data = reinterpret_cast<FindDialogData*>(cs->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data));

            // Create controls
            CreateWindowExW(0, L"STATIC", L"Search term:", WS_CHILD | WS_VISIBLE, 
                            15, 20, 90, 20, hWnd, NULL, cs->hInstance, NULL);

            data->hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", 
                                          WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 
                                          110, 17, 180, 22, hWnd, (HMENU)1001, cs->hInstance, NULL);

            CreateWindowExW(0, L"BUTTON", L"Find Next", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 
                            305, 15, 85, 24, hWnd, (HMENU)IDOK, cs->hInstance, NULL);

            CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
                            305, 45, 85, 24, hWnd, (HMENU)IDCANCEL, cs->hInstance, NULL);

            // Set system font for controls
            HFONT hSysFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            EnumChildWindows(hWnd, [](HWND child, LPARAM font) -> BOOL {
                SendMessageW(child, WM_SETFONT, (WPARAM)font, TRUE);
                return TRUE;
            }, (LPARAM)hSysFont);

            SetFocus(data->hEdit);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == IDOK) {
                if (data && data->hEdit) {
                    int len = GetWindowTextLengthW(data->hEdit);
                    if (len > 0) {
                        data->query.resize(len);
                        GetWindowTextW(data->hEdit, &data->query[0], len + 1);
                        data->confirmed = true;
                    }
                }
                DestroyWindow(hWnd);
            } 
            else if (wmId == IDCANCEL) {
                DestroyWindow(hWnd);
            }
            return 0;
        }

        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool FindDialog::show(HWND hwndParent, std::wstring& outQuery) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = FindDlgProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = L"PaperPadFindDlg";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    FindDialogData data;
    
    // Create the search dialog box centered over parent
    RECT rcParent = { 0 };
    GetWindowRect(hwndParent, &rcParent);
    int width = 420;
    int height = 120;
    int x = rcParent.left + (rcParent.right - rcParent.left - width) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - height) / 2;

    HWND hwndDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"PaperPadFindDlg",
        L"Find",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        x, y, width, height,
        hwndParent,
        NULL,
        hInstance,
        &data
    );

    if (!hwndDlg) {
        return false;
    }

    // Modal dialog disables the main app window
    EnableWindow(hwndParent, FALSE);
    ShowWindow(hwndDlg, SW_SHOW);
    UpdateWindow(hwndDlg);

    // Modal loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageW(hwndDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // Re-enable parent window
    EnableWindow(hwndParent, TRUE);
    SetFocus(hwndParent);

    if (data.confirmed) {
        outQuery = data.query;
        return true;
    }
    return false;
}

} // namespace PaperPad
