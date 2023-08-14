#pragma once

#include <wx/event.h>
#include <wx/timer.h>
#include <wx/stream.h>
#include <wx/webrequest.h>

class UpdateManager : public wxEvtHandler
{
public:
	struct VersionInfo {
		wxString title;
		wxString content;
		wxString url;
	};

public:
	UpdateManager();

	void GetUpdates(wxWindow* parent = nullptr, bool bHideDontShowCheck = false);
	bool UpdateEvents();

private:
	void OnTimer(wxTimerEvent& event);

	VersionInfo ParseXmlData(wxInputStream* stream);
	bool CheckVersion(wxString version);
	int ConvertVersion(wxString version);

	bool UpdateFile(const wxString& url);

public:
	static UpdateManager& GetInstance();
	static void Start();

private:
	wxTimer m_timer;
};

