#pragma once

#include "BaseRenderer.h"

class wxUxThemeHandle;

class DarkThemeRenderer : public BaseRenderer
{
	wxDECLARE_DYNAMIC_CLASS(DarkThemeRenderer);

public:
	virtual void OnInit();
	virtual void OnRelease();

	virtual bool IsSystemRender() const { return true; }

	virtual void InitWindow(wxWindow* window);
	virtual void InitButton(wxWindow* ctrl);
	virtual void InitComboBox(wxWindow* ctrl);
	virtual void InitEdit(wxWindow* ctrl);
	virtual void InitListBox(wxWindow* ctrl);
	virtual void InitCommonControl(wxWindow* ctrl);

	virtual wxColour GetBackgroundColor() { return wxColor(56, 56, 56); }
	virtual void DrawPushButton(wxWindow* control, wxDC& dc, const wxRect& rect, int OldState, int State);

private:
	void DrawButton(WXHWND hWnd, wxDC& dc, const wxString& label, const wxRect& rect, int State, wxUxThemeHandle& theme);
};

