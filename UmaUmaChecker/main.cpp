#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <tchar.h>
#include <gdiplus.h>
#include <Richedit.h>
#include <Commctrl.h>
#include <codecvt>
#include <regex>

#include "Uma.h"
#include "utility.h"
#include "Config.h"

#include "resource.h"



LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

Uma *g_umaMgr = nullptr;


int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASSEX wcx;
    LPCTSTR lpszClassName = TEXT("UmaUmaChecker");
    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR token;

    HMODULE hRichLib = LoadLibrary(TEXT("MsftEdit.dll"));
    if (!hRichLib) {
        return -1;
    }

    Gdiplus::GdiplusStartup(&token, &input, NULL);

    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = DLGWINDOWEXTRA;
    wcx.hInstance = hInst;
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = lpszClassName;
    wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wcx)) {
        MessageBox(NULL, TEXT("RegisterClassEx関数の呼び出しに失敗しました。"), TEXT("ウマウマチェッカー"), MB_OK | MB_ICONERROR);
        return -1;
    }

    g_umaMgr = new Uma();

    g_umaMgr->Init();

    HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)WndProc);
    if (!hWnd) {
        delete g_umaMgr;
        MessageBox(NULL, TEXT("ウィンドウの生成に失敗しました。"), TEXT("ウマウマチェッカー"), MB_OK | MB_ICONERROR);
        return -1;
    }

    g_umaMgr->SetNotifyTarget(hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_umaMgr->config.Save();
    delete g_umaMgr;
    Gdiplus::GdiplusShutdown(token);
    FreeLibrary(hRichLib);
    return msg.wParam;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static HBRUSH hGreen, hYellow, hRed;

    switch (msg) {
        case WM_CREATE:
            hGreen = CreateSolidBrush(RGB(200, 255, 150));
            hYellow = CreateSolidBrush(RGB(255, 240, 150));
            hRed = CreateSolidBrush(RGB(255, 200, 220));
            /*
            if (g_umaMgr->config.WindowX == 0 && g_umaMgr->config.WindowY == 0) {
                RECT MonitorRect;
                RECT WindowRect;

                SystemParametersInfo(SPI_GETWORKAREA, 0, &MonitorRect, 0);
                GetWindowRect(hWnd, &WindowRect);

                g_umaMgr->config.WindowX = (MonitorRect.right - (WindowRect.right - WindowRect.left)) / 2;
                g_umaMgr->config.WindowY = (MonitorRect.bottom - (WindowRect.bottom - WindowRect.top)) / 2;
            }
            SetWindowPos(hWnd, HWND_TOP, g_umaMgr->config.WindowX, g_umaMgr->config.WindowY, 0, 0, SWP_NOSIZE);
            */
            break;
        case WM_INITDIALOG:
            for (auto& chara : g_umaMgr->GetCharacters()) {
                SendDlgItemMessageW(hWnd, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)chara->Name.c_str());
            }
            break;
        case WM_CTLCOLORSTATIC:
            switch (GetDlgCtrlID((HWND)lp)) {
                case IDC_EDITCHOISE1:
                    SetBkColor((HDC)wp, RGB(200, 255, 150));
                    return (LRESULT)hGreen;
                case IDC_EDITCHOISE2:
                    SetBkColor((HDC)wp, RGB(255, 240, 150));
                    return (LRESULT)hYellow;
                case IDC_EDITCHOISE3:
                    SetBkColor((HDC)wp, RGB(255, 200, 220));
                    return (LRESULT)hRed;
            }

            SetBkMode(((HDC)wp), TRANSPARENT);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_BUTTONSTART:
                    if (SendDlgItemMessage(hWnd, IDC_BUTTONSTART, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                        SetDlgItemText(hWnd, IDC_BUTTONSTART, TEXT("停止"));
                        g_umaMgr->Start();
                    }
                    else {
                        SetDlgItemText(hWnd, IDC_BUTTONSTART, TEXT("スタート"));
                        g_umaMgr->Stop();
                    }
                    break;
                case IDC_COMBO1:
                    if (HIWORD(wp) == CBN_SELCHANGE) {
                        int idx = SendDlgItemMessage(hWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
                        if (idx != CB_ERR) {
                            wchar_t* str;
                            int len = SendDlgItemMessageW(hWnd, IDC_COMBO1, CB_GETLBTEXTLEN, idx, 0);

                            str = new wchar_t[len + 1];
                            SendDlgItemMessageW(hWnd, IDC_COMBO1, CB_GETLBTEXT, idx, (LPARAM)str);

                            g_umaMgr->SetTrainingCharacter(str);

                            delete[] str;
                        }
                    }
                    break;
                case IDC_BUTTONSCREENSHOT: {
                    Gdiplus::Bitmap* image = g_umaMgr->ScreenShot();
                    if (image) {
                        CLSID clsid;

                        GetEncoderClsid(L"image/png", &clsid);

                        std::wstring directory = utility::GetExeDirectory();

                        CreateDirectoryW((directory + L"\\screenshots\\").c_str(), NULL);

                        std::wstring savename = directory
                            + std::wstring(L"\\screenshots\\screenshot_")
                            + std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
                            + L".png";
                        
                        GetEncoderClsid(L"image/png", &clsid);
                        image->Save(savename.c_str(), &clsid);
                        delete image;
                    }
                    break;
                }
            }
            break;
        case WM_CHANGEUMAEVENT: {
            int ids[3] = { IDC_EDITCHOISE1, IDC_EDITCHOISE2, IDC_EDITCHOISE3 };
            int detailids[3] = { IDC_EDIT1, IDC_EDIT2, IDC_EDIT3 };
            if (g_umaMgr->CurrentEvent) {
                SetDlgItemTextW(hWnd, IDC_EDITEVENTNAME, g_umaMgr->EventName.c_str());
                for (int i = 0; i < g_umaMgr->CurrentEvent->Choises.size() && i < 3; i++) {
                    SetDlgItemTextW(hWnd, ids[i], g_umaMgr->CurrentEvent->Choises[i].Title.c_str());
                    SetDlgItemTextW(hWnd, detailids[i], utility::replace(g_umaMgr->CurrentEvent->Choises[i].Effect, L"\n", L"\r\n").c_str());
                }
            }
            else {
                SetDlgItemTextW(hWnd, IDC_EDITEVENTNAME, L"");
                for (int i = 0; i < 3; i++) {
                    SetDlgItemTextW(hWnd, ids[i], L"");
                    SetDlgItemTextW(hWnd, detailids[i], L"");
                }
            }
            break;
        }
        case WM_MOVE:
            g_umaMgr->config.WindowX = LOWORD(lp);
            g_umaMgr->config.WindowY = HIWORD(lp);
            break;
        case WM_DESTROY:
            DeleteObject(hGreen);
            DeleteObject(hYellow);
            DeleteObject(hRed);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wp, lp);
    }

    return 0;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;
    UINT  size = 0;

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
}