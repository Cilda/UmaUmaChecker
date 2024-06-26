#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <winsock2.h>
#include <gdiplus.h>
#include <VersionHelpers.h>

#include <wx/app.h>
#include <wx/log.h>
#include <wx/debugrpt.h>
#include <wx/ffile.h>
#include <wx/translation.h>
#include <wx/uilocale.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "Update/UpdateManager.h"

#include "Recognizer/Uma.h"
#include "UI/MainFrame.h"
#include "Config/Config.h"

#include "Log/Log.h"
#include "version.h"
#include "Tesseract/Tesseract.h"
#include "Data/EventLibrary.h"
#include "Capture/UmaWindowCapture.h"
#include "Language/Languages.h"

typedef HRESULT(_stdcall* SetThreadDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef BOOL(*SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT);
typedef void (WINAPI* RtlGetVersionFunc)(OSVERSIONINFOEXW*);
typedef BOOL(_stdcall* SetProcessDPIAwareFunc)(VOID);


class MyApp : public wxApp {
public:
	MyApp()
	{
		wxHandleFatalExceptions();
	}

	virtual bool OnInit()
	{
		try {
			Log::Create();

			if (!wxApp::OnInit()) return false;
			
			LOG_INFO << "/------------------------------------------------------------------------------------------/";
			LOG_INFO << app_name << L" " << app_version;

			Config* config = Config::GetInstance();
			config->Load();

			InitLanguageSupport();

#ifndef _DEBUG
			UpdateManager::GetInstance().UpdateEvents();
#endif
			if (!EventLib.Load()) {
				return false;
			}

			Tesseract::Initialize();

			Gdiplus::GdiplusStartup(&token, &input, NULL);
			wxInitAllImageHandlers();

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

	void InitLanguageSupport()
	{
		if (!wxUILocale::UseDefault()) {
			wxLogWarning(wxT("Failed to initialize the default system locale."));
		}

		Languages::InitializeLanguages();

		Languages::SetLang(Config::GetInstance()->Language);
	}

	virtual int OnExit()
	{
		Config* config = Config::GetInstance();
		config->Save();

		Gdiplus::GdiplusShutdown(token);
		Tesseract::Uninitialize();
		UmaWindowCapture::Uninitilize();
		Languages::UninitializeLanguages();

		return 0;
	}

#if _DEBUG
	virtual void OnFatalException()
	{
		GenerateReport(wxDebugReport::Context_Exception);
	}

	void GenerateReport(wxDebugReport::Context ctx)
	{
		wxDebugReportCompress* report = new wxDebugReportCompress();

		report->AddAll(ctx);

		if (wxDebugReportPreviewStd().Show(*report)) {
			if (report->Process()) {
				report->Reset();
			}
		}

		delete report;
	}
#endif

private:
	Gdiplus::GdiplusStartupInput input;
	ULONG_PTR token;
};

wxIMPLEMENT_APP(MyApp);
