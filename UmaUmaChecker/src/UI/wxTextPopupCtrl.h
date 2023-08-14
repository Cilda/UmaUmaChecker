#pragma once

#include <wx/popupwin.h>
#include <wx/panel.h>
#include <wx/textctrl.h>

#include "wxUmaTextCtrl.h"

class wxTextPopupCtrl : public wxPopupTransientWindow
{
public:
	wxTextPopupCtrl(wxWindow* parent, const wxSize& size);
	~wxTextPopupCtrl();

	virtual void Popup(wxWindow* focus = NULL);
	virtual void Dismiss();

	void SetText(const wxString& text);
	virtual bool ProcessLeftDown(wxMouseEvent& event);

private:
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnCtrlLeave(wxMouseEvent& event);

protected:
	virtual void OnDismiss();

private:
	wxWindow* m_panel;
	wxUmaTextCtrl* m_textCtrl;
};

