#include "AboutDialog.h"

#include <wx/sizer.h>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/listbox.h>

#include "Config.h"
#include "version.h"
#include "UpdateManager.h"

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

	wxHyperlinkCtrl* hyperLinkHomepage = new wxHyperlinkCtrl(this, wxID_ANY, wxT("https://github.com/Cilda/UmaUmaChecker"), wxT("https://github.com/Cilda/UmaUmaChecker"));
	sizerInfo->Add(hyperLinkHomepage, 0, wxALL | wxCENTER, 5);
	sizerTop->Add(sizerInfo, 0, wxALL, 20);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* text = new wxStaticText(this, wxID_ANY, wxT("以下のライブラリを含んでいます:"));
	sizer->Add(text, 0);

	wxListBox* listBox = new wxListBox(this, wxID_ANY);
	listBox->Append(wxT("Tesseract OCR"));
	listBox->Append(wxT("OpenCV"));
	listBox->Append(wxT("wxWidgets"));
	listBox->Append(wxT("nlohmann-json"));
	listBox->Append(wxT("simstring"));
	listBox->Append(wxT("Boost.Log"));
	listBox->Append(wxT("Boost.Locale"));
	sizer->Add(listBox, 0, wxEXPAND);

	sizerTop->Add(sizer, 0, (wxALL & ~wxTOP) | wxEXPAND, 5);

	// ボタン類
	wxStaticLine* staticLine = new wxStaticLine(this, wxID_ANY);
	sizerTop->Add(staticLine, 0, wxEXPAND, 5);

	wxBoxSizer* bSizerButtons = new wxBoxSizer(wxHORIZONTAL);
	m_buttonOk = new wxButton(this, wxID_OK);
	bSizerButtons->Add(m_buttonOk, 0, wxLEFT | wxRIGHT, 5);

	m_buttonUpdateCheck = new wxButton(this, wxID_ANY, wxT("更新の確認"));
	bSizerButtons->Add(m_buttonUpdateCheck, 0, wxLEFT | wxRIGHT, 5);

	sizerTop->Add(bSizerButtons, 0, wxALL | wxCENTER, 5);

	this->SetSizer(sizerTop);
	sizerTop->Fit(this);

	this->Centre(wxBOTH);

	m_buttonUpdateCheck->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AboutDialog::OnClickUpdateCheck, this);
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::OnClickUpdateCheck(wxCommandEvent& event)
{
	CheckUpdate();
}

void AboutDialog::CheckUpdate()
{
	auto& instance = UpdateManager::GetInstance();

	instance.GetUpdates(this, true);
}
