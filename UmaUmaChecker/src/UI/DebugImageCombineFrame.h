#pragma once

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/stattext.h>


namespace cv {
	class Mat;
}
namespace Gdiplus {
	class Bitmap;
}

class DebugImageCombineFrame : public wxFrame, public wxThreadHelper
{
public:
	DebugImageCombineFrame(wxWindow* parent);
	~DebugImageCombineFrame();

private:
	void OnDropFiles(wxDropFilesEvent& event);
	void OnClickStartCapture(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnThreadUpdate(wxThreadEvent& event);

	// wxThreadHelper を介して継承されました
	void* Entry() override;

private:
	wxTimer timer;
	wxStaticText* text;
	bool bRunning;

	DECLARE_EVENT_TABLE()
};

