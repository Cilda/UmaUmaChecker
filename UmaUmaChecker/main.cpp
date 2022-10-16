#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <winsock2.h>

#include <wx/app.h>
#include <gdiplus.h>
#include <VersionHelpers.h>

#include "Uma.h"
#include "MainFrame.h"
#include "Config.h"

typedef HRESULT(_stdcall* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef BOOL(*SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef void (WINAPI* RtlGetVersionFunc)(OSVERSIONINFOEXW*);
typedef BOOL(_stdcall* SetProcessDPIAwareFunc)(VOID);


class MyApp : public wxApp {
public:
	virtual bool OnInit()
	{
		if (!wxApp::OnInit()) return false;

		InitDPI();
		Gdiplus::GdiplusStartup(&token, &input, NULL);
		wxInitAllImageHandlers();

		Config* config = Config::GetInstance();
		config->Load();

		MainFrame* frame = new MainFrame(NULL);
		frame->Show(true);
		SetTopWindow(frame);

		return true;
	}

	virtual int OnExit()
	{
		Config* config = Config::GetInstance();
		config->Save();

		Gdiplus::GdiplusShutdown(token);
		return 0;
	}

	void InitDPI()
	{
		HMODULE hModule = GetModuleHandle(TEXT("user32"));
		if (hModule) {
			SetThreadDpiAwarenessContextFunc ThreadAwareFunc = (SetThreadDpiAwarenessContextFunc)GetProcAddress(hModule, "SetThreadDpiAwarenessContext");
			// Windows10 Anniversary Update以上
			if (ThreadAwareFunc) {
				HMODULE hNtModule = GetModuleHandleW(TEXT("ntdll"));
				if (hNtModule) {
					RtlGetVersionFunc GetVerFunc = (RtlGetVersionFunc)GetProcAddress(hNtModule, "RtlGetVersion");
					if (GetVerFunc) {
						OSVERSIONINFOEXW osw;

						ZeroMemory(&osw, sizeof(osw));
						osw.dwOSVersionInfoSize = sizeof(osw);

						GetVerFunc(&osw);

						// Windows10(1809)以上 or Windows11
						if (osw.dwMajorVersion == 10 && osw.dwBuildNumber >= 17763) {
							ThreadAwareFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2); // DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED
						}
						// Windows10 Creators Update以上 or Windows11
						else if (osw.dwMajorVersion == 10 && osw.dwBuildNumber >= 15063) {
							ThreadAwareFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
						}
						// Windows10 Creators Update未満
						else {
							ThreadAwareFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
						}
					}
				}
			}
			else {
				SetProcessDPIAwareFunc ProcessAwareFunc = (SetProcessDPIAwareFunc)GetProcAddress(hModule, "SetProcessDPIAware");
				if (ProcessAwareFunc) ProcessAwareFunc();
			}
		}
	}

private:
	Gdiplus::GdiplusStartupInput input;
	ULONG_PTR token;
};

wxIMPLEMENT_APP(MyApp);
