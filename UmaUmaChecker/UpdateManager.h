#pragma once

#include <wx/event.h>
#include <wx/timer.h>
#include <wx/stream.h>

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

	void GetUpdates();

private:
	void OnTimer(wxTimerEvent& event);

	VersionInfo ParseXmlData(wxInputStream* stream);

public:
	static UpdateManager& GetInstance();
	static void Start();

private:
	wxTimer m_timer;
};

