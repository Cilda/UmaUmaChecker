#include "CheckUpdateDialog.h"

#include <wx/sizer.h>
#include <wx/html/htmlwin.h>
#include <wx/button.h>
#include <wx/checkbox.h>

#include "Config/Config.h"

enum {
	DontShow = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE(CheckUpdateDialog, wxDialog)
	EVT_CHECKBOX(DontShow, CheckUpdateDialog::OnDontShow)
	EVT_BUTTON(wxID_YES, CheckUpdateDialog::OnClickUpdate)
wxEND_EVENT_TABLE()


CheckUpdateDialog::CheckUpdateDialog(wxWindow* parent, UpdateManager::VersionInfo* version, bool bHideDontShowCheck) : wxDialog(parent, wxID_ANY, wxT("ウマウマチェッカー を更新"))
{
	url = version->url;

	wxBoxSizer* sizer = new	wxBoxSizer(wxVERTICAL);
	{
		{
			wxHtmlWindow* html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
			wxString content;

			content << wxString::Format(wxT("<h1>%s に更新できます</h1><h2>更新履歴</h2>"), version->title);
			content << version->content;

			html->SetPage(content);
			sizer->Add(html, 0, wxALL | wxEXPAND, 5);
		}
		{
			wxBoxSizer* stdbutton = new wxBoxSizer(wxHORIZONTAL);

			wxCheckBox* checkbox = new wxCheckBox(this, DontShow, wxT("起動時に確認をしない"));
			if (bHideDontShowCheck) checkbox->Hide();
			stdbutton->Add(checkbox, 0, wxALIGN_CENTER);

			stdbutton->Add(0, 0, 1);

			wxButton* buttonSkip = new wxButton(this, wxID_CANCEL, wxT("スキップ"));
			stdbutton->Add(buttonSkip, 0);

			wxButton* buttonUpdate = new wxButton(this, wxID_YES, wxT("更新する"));
			stdbutton->Add(buttonUpdate, 0);

			sizer->Add(stdbutton, 0, wxALL | wxEXPAND, 5);
		}
	}

	this->SetSizer(sizer);
	this->Layout();
	this->Fit();

	this->Centre();
}

CheckUpdateDialog::~CheckUpdateDialog()
{
}

void CheckUpdateDialog::OnDontShow(wxCommandEvent& event)
{
	Config* config = Config::GetInstance();

	config->EnableCheckUpdate = !event.IsChecked();
}

void CheckUpdateDialog::OnClickUpdate(wxCommandEvent& event)
{
	ShellExecute(this->GetHWND(), TEXT("open"), url.tchar_str<TCHAR>(), NULL, NULL, NULL);
	this->EndModal(wxID_YES);
}
