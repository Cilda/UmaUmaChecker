#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <tchar.h>
#include <gdiplus.h>
#include <Richedit.h>
#include <Commctrl.h>
#include "Uma.h"
#include "utility.h"

#include <tesseract/baseapi.h>
#include <codecvt>
#include "resource.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);


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

    Uma* uma = new Uma();

    HWND hWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, NULL, (LPARAM)uma);
    if (!hWnd) {
        MessageBox(NULL, TEXT("ウィンドウの生成に失敗しました。"), TEXT("ウマウマチェッカー"), MB_OK | MB_ICONERROR);
        return -1;
    }

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)uma);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete uma;
    Gdiplus::GdiplusShutdown(token);
    FreeLibrary(hRichLib);
    return msg.wParam;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static HBRUSH hGreen, hYellow, hRed;
    Uma* uma = (Uma*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (msg) {
        case WM_CREATE:
            hGreen = CreateSolidBrush(RGB(200, 255, 150));
            hYellow = CreateSolidBrush(RGB(255, 240, 150));
            hRed = CreateSolidBrush(RGB(255, 200, 220));
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
            switch (wp) {
                case IDC_BUTTONSTART:
                    uma->Start();
                    break;
                case IDC_BUTTONSCREENSHOT: {
                    Gdiplus::Bitmap* image = uma->ScreenShot();
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