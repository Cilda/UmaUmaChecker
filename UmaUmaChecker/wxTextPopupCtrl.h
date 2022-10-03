#pragma once

#include <wx/frame.h>
#include <wx/textctrl.h>

class wxTextPopupCtrl : public wxFrame
{
public:
	wxTextPopupCtrl(wxWindow* parent, wxPoint pos, wxSize size);
	~wxTextPopupCtrl();

private:
	void OnLeave(wxMouseEvent& event);

public:
	wxTextCtrl* m_textCtrl;
};

