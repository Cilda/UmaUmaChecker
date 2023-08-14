#pragma once

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/stattext.h>

#include "Recognizer/CombineImage.h"

namespace cv {
	class Mat;
}
namespace Gdiplus {
	class Bitmap;
}

class DebugImageCombineFrame : public wxFrame
{
public:
	DebugImageCombineFrame(wxWindow* parent);
	~DebugImageCombineFrame();

private:
	void OnDropFiles(wxDropFilesEvent& event);
	void OnClickStartCapture(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxTimer timer;
	CombineImage combine;
	wxStaticText* text;
	std::thread thread;
	bool bRunning;
};

