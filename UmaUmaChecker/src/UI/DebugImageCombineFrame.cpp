#include "DebugImageCombineFrame.h"

#include <thread>
#include <future>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <opencv2/opencv.hpp>

#include "Recognizer/CombineImage.h"
#include "Recognizer/ScrollbarDetector.h"
#include "Capture/UmaWindowCapture.h"


enum {
	ID_StartCapture = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE(DebugImageCombineFrame, wxFrame)
	EVT_DROP_FILES(DebugImageCombineFrame::OnDropFiles)
	EVT_BUTTON(ID_StartCapture, DebugImageCombineFrame::OnClickStartCapture)
	EVT_CLOSE(DebugImageCombineFrame::OnClose)
wxEND_EVENT_TABLE()

DebugImageCombineFrame::DebugImageCombineFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, wxT("画像結合"))
{
	this->DragAcceptFiles(true);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	text = new wxStaticText(this, wxID_ANY, wxT(""));
	auto button = new wxButton(this, ID_StartCapture, wxT("Start Capture"));

	sizer->Add(text);
	sizer->Add(button);
	this->SetSizer(sizer);

	timer.Bind(wxEVT_TIMER, &DebugImageCombineFrame::OnTimer, this);
	timer.Start(10);
	
	FILE* fp;
	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w", stdout);
}

DebugImageCombineFrame::~DebugImageCombineFrame()
{
	FreeConsole();
}

void DebugImageCombineFrame::OnDropFiles(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() == 0) return;

	wxString filename = event.GetFiles()[0];
}

void DebugImageCombineFrame::OnClickStartCapture(wxCommandEvent& event)
{
	if (combine.IsCapturing()) {
		combine.EndCapture();
	}
	else {
		bRunning = true;
		thread = std::thread([this] {
			combine.StartCapture();
			bRunning = false;
		});

		while (bRunning) {
			wxYield();
		}

		if (thread.joinable()) thread.join();
	}
}

void DebugImageCombineFrame::OnTimer(wxTimerEvent& event)
{
	if (!combine.IsCapturing()) {
		text->SetLabel(wxT("キャプチャしていません。"));
	}
	else {
		int msec = combine.GetProgressTime();
		if (msec >= 10) text->SetLabel(wxString::Format(wxT("処理時間:%d"), msec));
	}
}

void DebugImageCombineFrame::OnClose(wxCloseEvent& event)
{
	if (thread.joinable()) {
		wxMessageBox(wxT("終了するには結合を停止してください。"));
		return;
	}

	Destroy();
}
