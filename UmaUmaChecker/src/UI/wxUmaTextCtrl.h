#pragma once

#include <wx/textctrl.h>

class wxUmaTextCtrl : public wxTextCtrl
{
public:
	wxUmaTextCtrl(wxWindow* parent, int line = -1);
	~wxUmaTextCtrl();

	virtual void SetValue(const wxString& value);
	virtual void AppendText(const wxString& text);

	virtual bool SetFont(const wxFont& font);
	void SetHeightByLine(int line);

private:
	int GetHeightByLine(int line) const;

private:
	int Line;
};

