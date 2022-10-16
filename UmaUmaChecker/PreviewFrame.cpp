#include "PreviewFrame.h"

#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

wxDEFINE_EVENT(DROP_IMAGE, wxCommandEvent);


PreviewFrame::PreviewFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, wxT("プレビュー"), wxDefaultPosition, wxSize(400, 500), wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	this->Layout();
	this->Centre(wxBOTH);

	this->DragAcceptFiles(true);

	this->Bind(wxEVT_CLOSE_WINDOW, &PreviewFrame::OnClose, this);
	this->Bind(wxEVT_DROP_FILES, &PreviewFrame::OnDropFiles, this);
	this->Bind(wxEVT_PAINT, &PreviewFrame::OnPaint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &PreviewFrame::OnEraseBackground, this);
}

PreviewFrame::~PreviewFrame()
{
}

void PreviewFrame::SetImage(WXHBITMAP hBmp, int width, int height)
{
	image.InitFromHBITMAP(hBmp, width, height, 32);
	this->Refresh();
}

void PreviewFrame::OnClose(wxCloseEvent& event)
{
	Hide();
}

void PreviewFrame::OnDropFiles(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() > 0) {
		wxString filename = event.GetFiles()[0];

		if (wxFileExists(filename)) {
			image = wxBitmap(filename, wxBITMAP_TYPE_ANY);
			this->Refresh();

			wxCommandEvent event(DROP_IMAGE, GetId());
			ProcessWindowEvent(event);
		}
	}
}

void PreviewFrame::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);

	dc.Clear();
	
	if (image.IsOk()) {
		wxSize cSize = this->GetClientSize();
		wxSize iSize = image.GetSize();

		dc.Clear();

		float xscale = (float)cSize.x / (float)iSize.x;
		float yscale = (float)cSize.y / (float)iSize.y;
		float scale = fmin(xscale, yscale);

		if (scale < 1.0) dc.SetUserScale(scale, scale);
		dc.DrawBitmap(image, 0, 0);
	}
}

void PreviewFrame::OnEraseBackground(wxEraseEvent& event)
{
}
