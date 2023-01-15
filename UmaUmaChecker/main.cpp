#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <winsock2.h>

#include <wx/app.h>
#include <gdiplus.h>
#include <VersionHelpers.h>

#include "UpdateManager.h"

#include "Uma.h"
#include "MainFrame.h"
#include "Config.h"

#include "Log.h"
#include "version.h"

typedef HRESULT(_stdcall* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef BOOL(*SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef void (WINAPI* RtlGetVersionFunc)(OSVERSIONINFOEXW*);
typedef BOOL(_stdcall* SetProcessDPIAwareFunc)(VOID);


class MyApp : public wxApp {
public:
	virtual bool OnInit()
	{
		if (!wxApp::OnInit()) return false;

		LOG_INFO << "/------------------------------------------------------------------------------------------/";
		LOG_INFO << app_name << L" " << app_version;

		try {
			Gdiplus::GdiplusStartup(&token, &input, NULL);
			wxInitAllImageHandlers();

			Config* config = Config::GetInstance();
			config->Load();

			MainFrame* frame = new MainFrame(NULL);
			frame->Show(true);
			SetTopWindow(frame);

			UpdateManager::Start();
		}
		catch (std::exception& ex) {
			LOG_EXCEPTION << ex.what();
		}

		return true;
	}

	virtual int OnExit()
	{
		Config* config = Config::GetInstance();
		config->Save();

		Gdiplus::GdiplusShutdown(token);
		return 0;
	}

private:
	Gdiplus::GdiplusStartupInput input;
	ULONG_PTR token;
};

wxIMPLEMENT_APP(MyApp);
