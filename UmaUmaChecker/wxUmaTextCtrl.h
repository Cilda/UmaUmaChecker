#pragma once

#include <wx/textctrl.h>

class wxUmaTextCtrl : public wxTextCtrl
{
public:
	wxUmaTextCtrl(wxWindow* parent);
	~wxUmaTextCtrl();

	virtual void SetValue(const wxString& value);
	virtual void AppendText(const wxString& text);
};
