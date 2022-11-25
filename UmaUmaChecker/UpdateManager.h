#pragma once

#include <wx/event.h>
#include <wx/timer.h>

class UpdateManager : public wxEvtHandler
{
public:
	UpdateManager();

	void GetUpdates();

private:
	void OnTimer(wxTimerEvent& event);

public:
	static UpdateManager& GetInstance();
	static void Start();

private:
	wxTimer m_timer;
};

