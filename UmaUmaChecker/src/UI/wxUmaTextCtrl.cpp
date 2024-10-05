#include "wxUmaTextCtrl.h"

#include <Windows.h>
#include <Richedit.h>
#include <wx/dcclient.h>
#include <wx/richtext/richtextbuffer.h>

wxUmaTextCtrl::wxUmaTextCtrl(wxWindow* parent, int line): wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_RICH2 | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxTE_DONTWRAP)
{
	Line = line;

	DWORD dwOptions = SendMessage(this->GetHWND(), EM_GETLANGOPTIONS, 0, 0);
	dwOptions &= ~IMF_AUTOFONT;
	dwOptions |= IMF_UIFONTS;
	SendMessage(this->GetHWND(), EM_SETLANGOPTIONS, 0, (LPARAM)dwOptions);
	SendMessage(this->GetHWND(), EM_SHOWSCROLLBAR, SB_HORZ, FALSE);

	SetHeightByLine(Line);
	SetValue("+1\n2\n3\n4\n5\n6");

	this->Refresh();
}

wxUmaTextCtrl::~wxUmaTextCtrl()
{
}

void wxUmaTextCtrl::SetValue(const wxString& value)
{
	wchar_t prev_c = 0;
	bool is_num = false;
	int or_check = 0;
	wxString text;
	wxColour DefaultColor = this->GetForegroundColour();
	wxColour color = DefaultColor;

	this->Clear();
	this->SetDefaultStyle(wxTextAttr(color, this->GetBackgroundColour()));

	for (wxString::const_iterator itr = value.begin(); itr != value.end(); itr++) {
		wchar_t c = *itr;

		if (or_check == 2) {
			or_check = 0;
			this->SetDefaultStyle(DefaultColor);
			this->AppendText(text);
			text = wxEmptyString;
		}

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

			this->SetDefaultStyle(DefaultColor);
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

			if (!is_num) {
				if (or_check == 0 && c == 'o') {
					or_check = 1;
				}
				else if (c == 'r' && or_check == 1) {
					or_check = 2;
				}
				else {
					is_num = false;
					color = DefaultColor;
				}
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

bool wxUmaTextCtrl::SetFont(const wxFont& font)
{
	bool ret = wxTextCtrl::SetFont(font);
	wxTextAttr attr = this->GetDefaultStyle();

	attr.SetFont(font);

	this->SetDefaultStyle(attr);
	
	wxString text = this->GetValue();

	this->SetValue(text);
	return ret;
}

void wxUmaTextCtrl::SetHeightByLine(int line)
{
	Line = line;

	wxClientDC dc(this);
	wxTextAttr attr = this->GetDefaultStyle();
	wxFontMetrics metrics = dc.GetFontMetrics();

	wxSize size = wxSize(-1, metrics.height * line);
	//this->SetMinClientSize(size);
	
	this->SetClientSize(size);
	this->SetMaxClientSize(size);
}

int wxUmaTextCtrl::GetHeightByLine(int line) const
{
	wxUmaTextCtrl* const self = const_cast<wxUmaTextCtrl*>(this);
	wxClientDC dc(self);
	wxTextAttr attr = this->GetDefaultStyle();
	wxFontMetrics metrics = dc.GetFontMetrics();

	return metrics.height * line;
}
