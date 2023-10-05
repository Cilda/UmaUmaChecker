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


CheckUpdateDialog::CheckUpdateDialog(wxWindow* parent, UpdateManager::VersionInfo* version, bool bHideDontShowCheck) : wxDialog(parent, wxID_ANY, _("Update UmaUmaChecker"))
{
	url = version->url;

	wxBoxSizer* sizer = new	wxBoxSizer(wxVERTICAL);
	{
		{
			wxHtmlWindow* html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(500, 300), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER);
			wxString content;

			content << wxString::Format(_("<h1>Available update %s</h1><h2>Update history</h2>"), version->title);
			content << version->content;

			html->SetPage(content);
			sizer->Add(html, 0, wxALL | wxEXPAND, 5);
		}
		{
			wxBoxSizer* stdbutton = new wxBoxSizer(wxHORIZONTAL);

			wxCheckBox* checkbox = new wxCheckBox(this, DontShow, _("Do not check on startup."));
			if (bHideDontShowCheck) checkbox->Hide();
			stdbutton->Add(checkbox, 0, wxALIGN_CENTER);

			stdbutton->Add(0, 0, 1);

			wxButton* buttonSkip = new wxButton(this, wxID_CANCEL, _("Skip"));
			stdbutton->Add(buttonSkip, 0);

			wxButton* buttonUpdate = new wxButton(this, wxID_YES, _("Update"));
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
