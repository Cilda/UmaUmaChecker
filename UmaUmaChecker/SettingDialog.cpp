#include "SettingDialog.h"

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>

#include "utility.h"

#include "Config.h"


SettingDialog::SettingDialog(wxWindow* parent, Config* config) : wxDialog(parent, wxID_ANY, wxT("設定"), wxDefaultPosition, wxSize(500, -1), wxDEFAULT_DIALOG_STYLE)
{
	this->bUpdated = false;
	this->config = config;

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sizeParent = new wxBoxSizer(wxVERTICAL);

	// 一般
	wxStaticBoxSizer* sizeS1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("一般")), wxVERTICAL);
	wxBoxSizer* sizeSB = new wxBoxSizer(wxHORIZONTAL);

	m_staticTextUpdate = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("イベント情報を最新に更新する"));
	m_buttonUpdate = new wxButton(sizeS1->GetStaticBox(), wxID_ANY, wxT("更新"));

	sizeSB->Add(m_staticTextUpdate, 0, wxALL, 5);
	sizeSB->Add(m_buttonUpdate, 0, wxALL, 5);

	sizeS1->Add(sizeSB, 1, wxALL, 5);
	sizeParent->Add(sizeS1, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);

	// スクリーンショット
	wxStaticBoxSizer* sizeS2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("スクリーンショット")), wxVERTICAL);
	wxFlexGridSizer* fgSize = new wxFlexGridSizer(0, 3, 0, 0);
	fgSize->SetFlexibleDirection(wxBOTH);
	fgSize->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);

	m_staticTextScreenShotPath = new wxStaticText(sizeS2->GetStaticBox(), wxID_ANY, wxT("場所:"));
	m_textCtrlScreenShotPath = new wxTextCtrl(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(280, -1));
	m_buttonBrowse = new wxButton(sizeS2->GetStaticBox(), wxID_ANY, wxT("参照"));

	fgSize->Add(m_staticTextScreenShotPath, 0, wxALL, 5);
	fgSize->Add(m_textCtrlScreenShotPath, 0, wxALL, 5);
	fgSize->Add(m_buttonBrowse, 0, wxALL, 5);

	sizeS2->Add(fgSize, 1, wxEXPAND, 5);
	sizeParent->Add(sizeS2, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// 高度な機能
	wxStaticBoxSizer* sizeS3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("高度な機能")), wxVERTICAL);
	m_checkDebugEnable = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("デバッグを有効にする"));
	m_checkDebugEnable->Enable(false);
	m_checkSaveScreenShot = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("イベントを識別できなかった場合にキャプチャする"));
	m_checkSaveScreenShot->Enable(false);

	sizeS3->Add(m_checkDebugEnable, 0, wxALL, 5);
	sizeS3->Add(m_checkSaveScreenShot, 0, wxALL, 5);

	sizeParent->Add(sizeS3, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// ダイアログボタン
	m_dialogButtonSizer = new wxStdDialogButtonSizer();
	wxButton* dbSizerOk = new wxButton(this, wxID_OK);
	m_dialogButtonSizer->SetAffirmativeButton(dbSizerOk);
	wxButton* dbSizerCancel = new wxButton(this, wxID_CANCEL, wxT("キャンセル"));
	m_dialogButtonSizer->SetCancelButton(dbSizerCancel);
	m_dialogButtonSizer->Realize();

	sizeParent->Add(m_dialogButtonSizer, 0, wxEXPAND | wxALL, 5);

	this->SetSizer(sizeParent);
	this->Layout();
	sizeParent->Fit(this);

	this->Centre(wxBOTH);

	this->Bind(wxEVT_INIT_DIALOG, &SettingDialog::OnInitDialog, this);
	m_buttonUpdate->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickUpdate, this);
	m_buttonBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickBrowse, this);
	dbSizerOk->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickOkButton, this);
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::OnInitDialog(wxInitDialogEvent& event)
{
	m_textCtrlScreenShotPath->SetLabelText(config->ScreenshotSavePath.c_str());
}

void SettingDialog::OnClickUpdate(wxCommandEvent& event)
{
	if (UpdateLibrary()) {
		bUpdated = true;
		wxMessageBox(wxT("更新が完了しました。"), wxT("ウマウマチェッカー"));
	}
}

void SettingDialog::OnClickBrowse(wxCommandEvent& event)
{
	wxDirDialog* dlg = new wxDirDialog(this, wxT("スクリーンショット保存先を選択"), wxT(""), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg->ShowModal() == wxID_OK) {
		m_textCtrlScreenShotPath->SetValue(dlg->GetPath());
	}
	dlg->Destroy();
}

void SettingDialog::OnClickOkButton(wxCommandEvent& event)
{
	config->ScreenshotSavePath = m_textCtrlScreenShotPath->GetValue().wc_str();
	this->EndModal(1);
}

bool SettingDialog::UpdateLibrary()
{
	wxWindowDisabler disabler;

	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Chara.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Events.json");
	
	for (auto& request : requests) {
		while (request.IsOk() && request.GetState() == wxWebRequest::State_Active) {
			wxYield();
		}
	}

	requests.clear();

	return true;
}

void SettingDialog::UpdateFile(const wxString& url)
{
	wxWebRequest request = wxWebSession::GetDefault().CreateRequest(this, url);
	
	if (!request.IsOk()) {
		return;
	}

	this->Bind(wxEVT_WEBREQUEST_STATE, [this](wxWebRequestEvent& event) {
		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				const wxWebResponse& response = event.GetResponse();
				if (response.GetStatus() == 200) {
					auto stream = response.GetStream();
					wxString path = utility::GetExeDirectory() + L"\\Library\\" + response.GetSuggestedFileName();
					wxFileOutputStream output(path);

					if (output.IsOk()) {
						output.Write(*stream);
						output.Close();
					}
				}
				break;
			}
			case wxWebRequest::State_Failed:
			case wxWebRequest::State_Cancelled:
				break;
		}
	});

	request.Start();

	requests.push_back(std::move(request));
}
