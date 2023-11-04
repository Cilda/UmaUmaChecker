#include "PreviewFrame.h"

#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>

#include "Config/Config.h"

wxDEFINE_EVENT(DROP_IMAGE, wxCommandEvent);


PreviewFrame::PreviewFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, _("Preview"), wxDefaultPosition, wxSize(400, 500), wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE)
{
	Config* config = Config::GetInstance();
	this->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	this->Layout();
	this->Centre(wxBOTH);

	this->DragAcceptFiles(true);

	m_popupMenu = new wxMenu();
	m_popupMenu->Append(wxID_CLEAR);
	m_popupMenu->AppendSeparator();
	m_popupMenu->Append(100, _("Save an image"));

	this->Bind(wxEVT_CLOSE_WINDOW, &PreviewFrame::OnClose, this);
	this->Bind(wxEVT_DROP_FILES, &PreviewFrame::OnDropFiles, this);
	this->Bind(wxEVT_PAINT, &PreviewFrame::OnPaint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &PreviewFrame::OnEraseBackground, this);
	this->Bind(wxEVT_RIGHT_DOWN, &PreviewFrame::OnRightButtonDown, this);

	this->Bind(wxEVT_MENU, &PreviewFrame::OnMenuSaveAs, this, 100);
	this->Bind(wxEVT_MENU, &PreviewFrame::OnMenuClear, this, wxID_CLEAR);
}

PreviewFrame::~PreviewFrame()
{
}

void PreviewFrame::SetImage(WXHBITMAP hBmp, int width, int height)
{
	WXHBITMAP hOldBmp = image.GetHBITMAP();
	if (hOldBmp) {
		DeleteObject(hOldBmp);
	}
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

void PreviewFrame::OnRightButtonDown(wxMouseEvent& event)
{
	m_popupMenu->Enable(100, image.IsOk());
	PopupMenu(m_popupMenu);
}

void PreviewFrame::OnMenuSaveAs(wxCommandEvent& event)
{
	if (!image.IsOk()) return;

	wxFileDialog SaveDlg(this, _("Save an image"), wxEmptyString, wxEmptyString, wxT("PNG (*.png)|*.png"), wxFD_SAVE| wxFD_OVERWRITE_PROMPT);
	if (SaveDlg.ShowModal() == wxID_CANCEL) return;

	image.SaveFile(SaveDlg.GetPath(), wxBITMAP_TYPE_PNG);
}

void PreviewFrame::OnMenuClear(wxCommandEvent& event)
{
	if (image.IsOk()) {
		image = wxBitmap();
		this->Refresh();
	}
}
