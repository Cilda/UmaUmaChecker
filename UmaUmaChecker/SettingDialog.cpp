#include "SettingDialog.h"

#include <vector>

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/file.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/valnum.h>

#include "utility.h"

#include "Config.h"
#include "../libwinrt/winrt_capture.h"


SettingDialog::SettingDialog(wxWindow* parent, Config* config) : wxDialog(parent, wxID_ANY, wxT("設定"), wxDefaultPosition, wxSize(500, -1), wxDEFAULT_DIALOG_STYLE)
{
	this->bUpdated = false;
	this->UpdatedCount = 0;
	this->config = config;

	this->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));
	this->SetDoubleBuffered(true);

	wxBoxSizer* sizeParent = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer* sizeS1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("一般")), wxVERTICAL);
		{
			wxFlexGridSizer* gridSizer = new wxFlexGridSizer(4, 2, 3, 0);
			{
				// 更新
				m_staticTextUpdate = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("イベントを最新に更新する"));
				m_buttonUpdate = new wxButton(sizeS1->GetStaticBox(), wxID_ANY, wxT("更新"));
				gridSizer->Add(m_staticTextUpdate, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_buttonUpdate, 0);

				// フォント選択
				{
					m_staticTextFontSelect = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("ウィンドウのフォント"));
					gridSizer->Add(m_staticTextFontSelect, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

					wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
					{
						m_comboFontList = new FontComboBox(sizeS1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT);
						sizer->Add(m_comboFontList, 0, wxRIGHT, 5);
					}
					{
						m_comboFontSizeList = new wxComboBox(sizeS1->GetStaticBox(), wxID_ANY);
						wxIntegerValidator<int> validator;
						validator.SetMin(2);
						validator.SetMax(100);
						m_comboFontSizeList->SetValidator(validator);
						std::vector<int> FontSize{ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 };
						for (auto size : FontSize) {
							m_comboFontSizeList->AppendString(wxString::Format(wxT("%d"), size));
						}
						sizer->Add(m_comboFontSizeList, 0, wxRIGHT, 5);
					}

					gridSizer->Add(sizer);
				}
				/*
				m_staticTextFontSelect = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("ウィンドウのフォント"));
				m_fontPickerCtrl = new wxFontPickerCtrl(sizeS1->GetStaticBox(), wxID_ANY, wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));
				gridSizer->Add(m_staticTextFontSelect, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_fontPickerCtrl);
				*/

				// 表示行数
				m_staticTextMaxLine = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("効果テキスト最大行数"));
				m_spinCtrlMaxLine = new wxSpinCtrl(sizeS1->GetStaticBox(), wxID_ANY);
				m_spinCtrlMaxLine->SetRange(2, 10);
				gridSizer->Add(m_staticTextMaxLine, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_spinCtrlMaxLine, 0);

				// テーマ
				wxStaticText* themeText = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("テーマ"));
				m_comboTheme = new wxComboBox(sizeS1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
				m_comboTheme->AppendString(wxT("ライト"));
				m_comboTheme->AppendString(wxT("ダーク"));
				gridSizer->Add(themeText, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_comboTheme, 0);

				sizeS1->Add(gridSizer, 1, wxLEFT | wxBOTTOM, 5);
			}

			// 選択肢表示チェック
			m_checkBoxHideOption = new wxCheckBox(sizeS1->GetStaticBox(), wxID_ANY, wxT("「選択肢なし」のイベントを表示しない"));
			sizeS1->Add(m_checkBoxHideOption, 0, wxLEFT | wxBOTTOM, 5);

			// ステータス表示
			m_checkBoxShowStatusBar = new wxCheckBox(sizeS1->GetStaticBox(), wxID_ANY, wxT("ステータスバーを表示する"));
			sizeS1->Add(m_checkBoxShowStatusBar, 0, wxLEFT | wxBOTTOM, 5);

			// 更新確認
			m_checkBoxCheckUpdate = new wxCheckBox(sizeS1->GetStaticBox(), wxID_ANY, wxT("起動時に更新がある場合は通知する"));
			sizeS1->Add(m_checkBoxCheckUpdate, 0, wxLEFT | wxBOTTOM, 5);

			sizeParent->Add(sizeS1, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// OCR設定
		wxStaticBoxSizer* sizeSystem = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("OCR設定")), wxVERTICAL);
		{
			wxFlexGridSizer* gridSizer = new wxFlexGridSizer(1, 2, 3, 0);
			{
				wxStaticText* Static = new wxStaticText(sizeSystem->GetStaticBox(), wxID_ANY, wxT("OCR数"));
				gridSizer->Add(Static, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
				{
					m_comboOcrPoolSize = new wxComboBox(sizeSystem->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
					for (int i = 1; i <= 10; i++) m_comboOcrPoolSize->AppendString(wxString::Format(wxT("%d"), i));
					s->Add(m_comboOcrPoolSize, 0);

					s->Add(new wxStaticText(sizeSystem->GetStaticBox(), wxID_ANY, wxT("※アプリケーションの再起動が必要です。")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
				}
				gridSizer->Add(s);
				sizeSystem->Add(gridSizer, 1, wxALL, 5);
				sizeSystem->Add(new wxStaticText(sizeSystem->GetStaticBox(), wxID_ANY, wxT("数値を大きくするとメモリとCPU使用率が増えますがイベント読み取り速度が向上します。\n数値を小さくするとメモリとCPU使用率は下がりますがイベント読み取りが遅くなります。")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
			}

			wxBoxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			{
				wxStaticText* Static = new wxStaticText(sizeSystem->GetStaticBox(), wxID_ANY, wxT("キャプチャ方法"));
				s2->Add(Static, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				m_comboCaptureMode = new wxComboBox(sizeSystem->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
				m_comboCaptureMode->AppendString(wxT("BitBlt"));
				if (winrt_capture_is_supported()) m_comboCaptureMode->AppendString(wxT("Windows 10 (1903以降)"));
				s2->Add(m_comboCaptureMode);

				sizeSystem->Add(s2, 1, wxALL, 5);
			}

			sizeParent->Add(sizeSystem, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// スクリーンショット
		wxStaticBoxSizer* sizeS2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("スクリーンショット")), wxVERTICAL);
		{
			wxFlexGridSizer* fgSize = new wxFlexGridSizer(2, 2, 3, 5);
			fgSize->AddGrowableCol(1);
			{
				m_staticTextScreenShotPath = new wxStaticText(sizeS2->GetStaticBox(), wxID_ANY, wxT("場所"));
				fgSize->Add(m_staticTextScreenShotPath, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				wxBoxSizer* sspathSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					m_textCtrlScreenShotPath = new wxTextCtrl(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(280, -1)));
					sspathSizer->Add(m_textCtrlScreenShotPath, 1);

					m_buttonBrowse = new wxButton(sizeS2->GetStaticBox(), wxID_ANY, wxT("参照"));
					sspathSizer->Add(m_buttonBrowse);
				}

				fgSize->Add(sspathSizer, 0, wxEXPAND);
				fgSize->Add(new wxStaticText(sizeS2->GetStaticBox(), wxID_ANY, wxT("種類")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				m_comboFileType = new wxComboBox(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
				m_comboFileType->Append(wxT("PNG"));
				m_comboFileType->Append(wxT("JPEG"));
				m_comboFileType->SetSelection(0);
				fgSize->Add(m_comboFileType, 0);
			}

			sizeS2->Add(fgSize, 1, wxEXPAND | wxALL, 5);
			sizeParent->Add(sizeS2, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// 高度な機能
		wxStaticBoxSizer* sizeS3 = new wxStaticBoxSizer(wxVERTICAL, this, wxT("デバッグモード"));
		{
			m_checkDebugEnable = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("デバッグを有効にする"));
			m_checkDebugEnable->Enable(false);
			m_checkSaveScreenShot = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("イベントを識別できなかった場合にキャプチャする"));

			sizeS3->Add(m_checkDebugEnable, 0, wxALL, 5);
			sizeS3->Add(m_checkSaveScreenShot, 0, wxALL, 5);

			sizeParent->Add(sizeS3, 0, wxEXPAND | wxALL, 5);
		}

		// ダイアログボタン
		m_dialogButtonSizer = new wxStdDialogButtonSizer();
		wxButton* dbSizerOk = new wxButton(this, wxID_OK);
		m_dialogButtonSizer->SetAffirmativeButton(dbSizerOk);
		wxButton* dbSizerCancel = new wxButton(this, wxID_CANCEL, wxT("キャンセル"));
		m_dialogButtonSizer->SetCancelButton(dbSizerCancel);
		m_dialogButtonSizer->Realize();

		sizeParent->Add(m_dialogButtonSizer, 0, wxEXPAND | wxALL, 5);

		dbSizerOk->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickOkButton, this);
	}
	this->SetSizer(sizeParent);
	this->Layout();
	this->Fit();
	this->Centre(wxBOTH);

	this->Bind(wxEVT_INIT_DIALOG, &SettingDialog::OnInitDialog, this);
	m_buttonUpdate->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickUpdate, this);
	m_buttonBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickBrowse, this);
	m_comboTheme->Bind(wxEVT_COMBOBOX, &SettingDialog::OnComboTheme, this);
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::OnInitDialog(wxInitDialogEvent& event)
{
	m_textCtrlScreenShotPath->SetLabelText(config->ScreenshotSavePath.c_str());
	m_checkBoxHideOption->Set3StateValue(config->IsHideNoneChoise ? wxCHK_CHECKED : wxCHK_UNCHECKED);
	m_checkBoxShowStatusBar->Set3StateValue(config->IsShowStatusBar ? wxCHK_CHECKED : wxCHK_UNCHECKED);
	m_checkSaveScreenShot->Set3StateValue(config->SaveMissingEvent ? wxCHK_CHECKED : wxCHK_UNCHECKED);
	m_checkDebugEnable->Set3StateValue(config->EnableDebug ? wxCHK_CHECKED : wxCHK_UNCHECKED);
	m_spinCtrlMaxLine->SetValue(config->OptionMaxLine);
	m_comboFileType->SetSelection(config->ImageType);
	m_checkBoxCheckUpdate->Set3StateValue(config->EnableCheckUpdate ? wxCHK_CHECKED : wxCHK_UNCHECKED);
	m_comboOcrPoolSize->SetStringSelection(wxString::Format(wxT("%d"), config->OcrPoolSize));
	m_comboTheme->SetSelection(config->Theme);
	m_comboFontList->SetStringSelection(config->FontName);
	m_comboFontSizeList->SetValue(wxString::Format(wxT("%d"), config->FontSize));
	m_comboCaptureMode->SetSelection(config->CaptureMode);
}

void SettingDialog::OnClickUpdate(wxCommandEvent& event)
{
	if (UpdateLibrary()) {
		bUpdated = true;
		wxMessageBox(wxT("更新が完了しました。"), wxT("ウマウマチェッカー"), wxICON_INFORMATION);
		return;
	}

	bUpdated = false;
	wxMessageBox(wxT("更新はありません。"), wxT("ウマウマチェッカー"), wxICON_INFORMATION);
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
	config->FontName = m_comboFontList->GetValue();
	m_comboFontSizeList->GetValue().ToInt(&config->FontSize);
	config->IsHideNoneChoise = m_checkBoxHideOption->IsChecked();
	config->IsShowStatusBar = m_checkBoxShowStatusBar->IsChecked();
	config->EnableDebug = m_checkDebugEnable->IsChecked();
	config->SaveMissingEvent = m_checkSaveScreenShot->IsChecked();
	config->OptionMaxLine = m_spinCtrlMaxLine->GetValue();
	config->ImageType = m_comboFileType->GetCurrentSelection();
	config->EnableCheckUpdate = m_checkBoxCheckUpdate->IsChecked();
	m_comboOcrPoolSize->GetStringSelection().ToInt(&config->OcrPoolSize);
	config->Theme = m_comboTheme->GetSelection();
	config->CaptureMode = m_comboCaptureMode->GetSelection();

	this->GetParent()->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));
	this->EndModal(1);
}

void SettingDialog::OnClickFontSelect(wxCommandEvent& event)
{
}

void SettingDialog::OnComboTheme(wxCommandEvent& event)
{
}

bool SettingDialog::UpdateLibrary()
{
	wxWindowDisabler disabler;
	UpdatedCount = 0;

	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Chara.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Events.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/ReplaceText.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/ScenarioEvents.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Skills.json");
	
	for (auto& request : requests) {
		while (request.IsOk() && request.GetState() == wxWebRequest::State_Active) {
			wxYield();
		}
	}

	requests.clear();

	return UpdatedCount > 0;
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
					int size = 0;

					if (wxFile::Exists(path)) {
						wxFile file(path);
						if (file.IsOpened()) {
							size = file.SeekEnd();
							file.Close();
						}
					}

					if (size != stream->GetSize()) {
						wxFileOutputStream output(path);

						if (output.IsOk()) {
							output.Write(*stream);
							UpdatedCount++;
							output.Close();
						}
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
