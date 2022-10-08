#include "AboutDialog.h"

#include <wx/sizer.h>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/stattext.h>

#include "version.h"


AboutDialog::AboutDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxT("ウマウマチェッカーについて"))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerInfo = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* sizerTitle = new wxBoxSizer(wxHORIZONTAL);
	m_bitmap = new wxStaticBitmap(this, wxID_ANY, wxNullIcon, wxDefaultPosition, wxSize(32, 32), wxBORDER);
	sizerTitle->Add(m_bitmap, 0, wxRIGHT, 5);

	wxStaticText* staticTextTitle = new wxStaticText(this, wxID_ANY, wxT("ウマウマチェッカー"));
	staticTextTitle->Wrap(-1);
	staticTextTitle->SetFont(wxFont(14, wxFONTFAMILY_SCRIPT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));
	sizerTitle->Add(staticTextTitle);

	sizerInfo->Add(sizerTitle, 0, wxALL | wxCENTER, 5);

	wxStaticText* staticTextVersion = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("%s\n%s"), app_version, app_copyright), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
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

	m_buttonLicense = new wxButton(this, wxID_ANY, wxT("ライセンス"));
	m_buttonLicense->Disable();
	bSizerButtons->Add(m_buttonLicense, 0, wxLEFT | wxRIGHT, 5);

	sizerTop->Add(bSizerButtons, 0, wxALL | wxCENTER, 5);

	this->SetSizer(sizerTop);
	this->Layout();
	sizerTop->Fit(this);

	this->Centre(wxBOTH);
}

AboutDialog::~AboutDialog()
{
}
