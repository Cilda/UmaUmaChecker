#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <winsock2.h>

#include "Uma.h"
#include "MainFrame.h"

#include <wx/app.h>
#include <gdiplus.h>

class MyApp : public wxApp {
public:
	virtual bool OnInit() {
		Gdiplus::GdiplusStartup(&token, &input, NULL);
		wxInitAllImageHandlers();

		MainFrame* frame = new MainFrame(NULL);
		frame->Init();
		frame->Show(true);
		SetTopWindow(frame);
		return true;
	}

	virtual int OnExit() {
		Gdiplus::GdiplusShutdown(token);
		return 0;
	}

private:
	Gdiplus::GdiplusStartupInput input;
	ULONG_PTR token;
};

wxIMPLEMENT_APP(MyApp);
