#pragma once

#include <wx/frame.h>
#include <wx/statbmp.h>
#include <wx/menu.h>

wxDECLARE_EVENT(DROP_IMAGE, wxCommandEvent);

class PreviewFrame : public wxFrame
{
public:
	PreviewFrame(wxWindow* parent);
	~PreviewFrame();

	const wxBitmap& GetImage() const { return image; }

	void SetImage(WXHBITMAP hBmp, int width, int height);

private:
	void OnClose(wxCloseEvent& event);
	void OnDropFiles(wxDropFilesEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnRightButtonDown(wxMouseEvent& event);
	void OnMenuSaveAs(wxCommandEvent& event);
	void OnMenuClear(wxCommandEvent& event);

private:
	wxBitmap image;
	wxMenu* m_popupMenu;
};

