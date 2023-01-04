#pragma once

#include <wx/window.h>
#include <wx/object.h>

class BaseRenderer : public wxObject
{
	wxDECLARE_ABSTRACT_CLASS(BaseRenderer);

public:
	virtual void OnInit() = 0;
	virtual void OnRelease() = 0;

	virtual bool IsSystemRender() const = 0;

	virtual void InitWindow(wxWindow* window) {}
	virtual void InitButton(wxWindow* ctrl) {}
	virtual void InitComboBox(wxWindow* ctrl) {}
	virtual void InitEdit(wxWindow* ctrl) {}
	virtual void InitCommonControl(wxWindow* ctrl) {}

	virtual wxColour GetBackgroundColor() = 0;
	virtual void DrawPushButton(wxWindow* control, wxDC& dc, const wxRect& rect, int oldState, int state) = 0;
	//virtual void DrawComboBox(wxWindow* control, wxDC& dc, const wxRect& rect) = 0;

	static void AllowDarkMode(WXHWND hWnd, bool bAllow);
};

