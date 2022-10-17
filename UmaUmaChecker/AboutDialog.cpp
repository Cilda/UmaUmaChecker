#include "AboutDialog.h"

#include <wx/sizer.h>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

#include "Config.h"
#include "version.h"


AboutDialog::AboutDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxT("ウマウマチェッカーについて"))
{
	Config* config = Config::GetInstance();
	this->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerInfo = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* sizerTitle = new wxBoxSizer(wxHORIZONTAL);
	m_bitmap = new wxStaticBitmap(this, wxID_ANY, wxICON(AppIcon), wxDefaultPosition, wxSize(32, 32));
	sizerTitle->Add(m_bitmap, 0, wxRIGHT, 5);

	wxStaticText* staticTextTitle = new wxStaticText(this, wxID_ANY, app_title);
	staticTextTitle->Wrap(-1);
	staticTextTitle->SetFont(wxFont(20, wxFONTFAMILY_SCRIPT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT("Yu Gothic UI")));
	sizerTitle->Add(staticTextTitle);

	sizerInfo->Add(sizerTitle, 0, wxALL | wxCENTER, 5);

	wxStaticText* staticTextVersion = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("%s"), app_copyright), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	staticTextVersion->Wrap(-1);
	sizerInfo->Add(staticTextVersion, 0, wxALL | wxCENTER, 5);

	/*
	wxStaticText* staticTextCopyright = new wxStaticText(this, wxID_ANY, wxT("Copyright 2022 Cilda"));
	staticTextCopyright->Wrap(-1);
	sizerInfo->Add(staticTextCopyright, 0, wxALL | wxCENTER, 5);
	*/

	wxHyperlinkCtrl* hyperLinkHomepage = new wxHyperlinkCtrl(this, wxID_ANY, wxT("https://github.com/Cilda/UmaUmaChecker"), wxT("https://github.com/Cilda/UmaUmaChecker"));
	sizerInfo->Add(hyperLinkHomepage, 0, wxALL | wxCENTER, 5);

	sizerTop->Add(sizerInfo, 0, wxALL, 20);

	// ボタン類
	wxStaticLine* staticLine = new wxStaticLine(this, wxID_ANY);
	sizerTop->Add(staticLine, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerButtons = new wxBoxSizer(wxHORIZONTAL);
	m_buttonOk = new wxButton(this, wxID_OK);
	bSizerButtons->Add(m_buttonOk, 0, wxLEFT | wxRIGHT, 5);

	m_buttonLicense = new wxButton(this, wxID_ANY, wxT("クレジット"));
	m_buttonLicense->Disable();
	bSizerButtons->Add(m_buttonLicense, 0, wxLEFT | wxRIGHT, 5);

	m_buttonUpdateCheck = new wxButton(this, wxID_ANY, wxT("更新の確認"));
	bSizerButtons->Add(m_buttonUpdateCheck, 0, wxLEFT | wxRIGHT, 5);

	sizerTop->Add(bSizerButtons, 0, wxALL | wxCENTER, 5);

	this->SetSizer(sizerTop);
	this->Layout();
	sizerTop->Fit(this);

	this->Centre(wxBOTH);

	m_buttonUpdateCheck->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AboutDialog::OnClickUpdateCheck, this);
}

AboutDialog::~AboutDialog()
{
	if (UpdateRequest.IsOk()) {
		UpdateRequest.Cancel();

		while (UpdateRequest.IsOk()) {
			wxYield();
		}
	}
}

void AboutDialog::OnClickUpdateCheck(wxCommandEvent& event)
{
	wxWindowDisabler disabler;

	CheckUpdate();

	if (UpdateRequest.IsOk()) {
		while (UpdateRequest.IsOk()) {
			wxYield();
		}
	}
}

void AboutDialog::CheckUpdate()
{
	UpdateRequest = wxWebSession::GetDefault().CreateRequest(this, wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/version.txt"));

	if (!UpdateRequest.IsOk()) {
		return;
	}

	this->Bind(wxEVT_WEBREQUEST_STATE, [this](wxWebRequestEvent& event) {
		bool IsActive = false;

		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				const wxWebResponse& response = event.GetResponse();
				if (response.GetStatus() == 200) {
					auto stream = response.GetStream();
					size_t size = stream->GetSize();
					char* buf = new char[size + 1];

					stream->ReadAll(buf, size);

					buf[size] = '\0';
					wxString latestVersion(buf);
					delete[] buf;

					if (latestVersion != app_version) {
						wxMessageBox(wxT("最新バージョンへ更新できます。"), wxT("ウマウマチェッカー"));
					}
					else {
						wxMessageBox(wxT("最新バージョンです。"), wxT("ウマウマチェッカー"));
					}
				}
				break;
			}
			case wxWebRequest::State_Failed:
			case wxWebRequest::State_Cancelled:
				break;
			case wxWebRequest::State_Active:
				IsActive = true;
				break;
		}

		if (!IsActive) {
			UpdateRequest = wxWebRequest();
		}
	});

	UpdateRequest.Start();
}
