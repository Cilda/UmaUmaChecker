#include "wxUmaTextCtrl.h"

#include <Windows.h>
#include <Richedit.h>

wxUmaTextCtrl::wxUmaTextCtrl(wxWindow* parent) : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_RICH2 | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxTE_DONTWRAP)
{
	DWORD dwOptions = SendMessage(this->GetHWND(), EM_GETLANGOPTIONS, 0, 0);
	dwOptions &= ~IMF_AUTOFONT;
	SendMessage(this->GetHWND(), EM_SETLANGOPTIONS, 0, (LPARAM)dwOptions);
	SendMessage(this->GetHWND(), EM_SHOWSCROLLBAR, SB_HORZ, FALSE);
}

wxUmaTextCtrl::~wxUmaTextCtrl()
{
}

void wxUmaTextCtrl::SetValue(const wxString& value)
{
	wchar_t prev_c = 0;
	bool is_num = false;
	wxString text;
	wxColour color = *wxBLACK;

	this->Clear();
	this->SetDefaultStyle(color);

	for (wxString::const_iterator itr = value.begin(); itr != value.end(); itr++) {
		wchar_t c = *itr;

		switch (c) {
			case '+':
			case '-':
			case '~':
				this->SetDefaultStyle(color);
				this->AppendText(text);
				text = wxEmptyString;
				break;
		}

		if (c == '+') {
			color = wxColour(0, 204, 0);
		}
		else if (c == '-') {
			color = wxColour(204, 0, 0);
		}
		else if (c == '~') {
			this->SetDefaultStyle(color);
			this->AppendText(text);
			text = wxEmptyString;

			this->SetDefaultStyle(*wxBLACK);
			this->AppendText(c);
			continue;
		}
		else if (c >= '0' && c <= '9') {
			is_num = true;
		}
		else {
			if (is_num) {
				this->SetDefaultStyle(color);
				this->AppendText(text);
				text = wxEmptyString;
				is_num = false;
			}
			else {
				is_num = false;
				color = *wxBLACK;
			}
		}

		text += c;
	}

	this->SetDefaultStyle(color);
	this->AppendText(text);
	this->SetInsertionPoint(0);
}

void wxUmaTextCtrl::AppendText(const wxString& text)
{
	this->SetInsertionPointEnd();
	this->WriteText(text);
}
