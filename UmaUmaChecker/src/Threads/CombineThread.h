#pragma once

#include <wx/thread.h>
#include <wx/event.h>
#include <wx/window.h>


wxDECLARE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMPLETED, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMBINE_STARTED, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMBINE_WAIT, wxThreadEvent);

class CombineThread : public wxThread
{
public:
	CombineThread(wxWindow* parent);
	~CombineThread();

protected:
	// wxThread を介して継承されました
	void* Entry() override;

private:
	wxWindow* parent;
};

