#include "SettingDialog.h"

#include <vector>

#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/file.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/valnum.h>
#include <wx/translation.h>

#include "Update/UpdateManager.h"

#include "Utils/utility.h"
#include "Config/Config.h"
#include "../../../libwinrt/winrt_capture.h"

SettingDialog::SettingDialog(wxWindow* parent, Config* config) : ThemedWindowWrapper<wxDialog>(parent, wxID_ANY, _("Setting"), wxDefaultPosition, wxSize(500, -1), wxDEFAULT_DIALOG_STYLE)
{
	this->bUpdated = false;
	this->UpdatedCount = 0;
	this->config = config;

	this->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	//this->SetBackgroundColour(wxColour(255, 255, 255));
	this->SetDoubleBuffered(true);

	wxBoxSizer* sizeParent = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer* sizeS1 = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, _("General")), wxVERTICAL);
		{
			wxFlexGridSizer* gridSizer = new wxFlexGridSizer(5, 2, 3, 0);
			{
				// 更新
				m_staticTextUpdate = new ThemedWrapper<wxStaticText>(sizeS1->GetStaticBox(), wxID_ANY, _("Update Event Data"));
				m_buttonUpdate = new ThemedButtonWrapper<wxButton>(sizeS1->GetStaticBox(), wxID_ANY, _("Update"));
				gridSizer->Add(m_staticTextUpdate, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_buttonUpdate, 0);

				// フォント選択
				{
					m_staticTextFontSelect = new ThemedWrapper<wxStaticText>(sizeS1->GetStaticBox(), wxID_ANY, _("Window Font"));
					gridSizer->Add(m_staticTextFontSelect, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

					wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
					{
						m_comboFontList = new ThemedComboBoxWrapper<FontComboBox>(sizeS1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY | wxCB_SORT);
						sizer->Add(m_comboFontList, 0, wxRIGHT, 5);
					}
					{
						m_comboFontSizeList = new ThemedComboBoxWrapper<wxComboBox>(sizeS1->GetStaticBox(), wxID_ANY);
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
				m_staticTextMaxLine = new ThemedWrapper<wxStaticText>(sizeS1->GetStaticBox(), wxID_ANY, _("Max number of effect line"));
				m_spinCtrlMaxLine = new ThemedWrapper<wxSpinCtrl>(sizeS1->GetStaticBox(), wxID_ANY);
				m_spinCtrlMaxLine->SetRange(2, 10);
				gridSizer->Add(m_staticTextMaxLine, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_spinCtrlMaxLine, 0);

				// テーマ
				wxStaticText* themeText = new ThemedWrapper<wxStaticText>(sizeS1->GetStaticBox(), wxID_ANY, _("Theme"));
				m_comboTheme = new ThemedComboBoxWrapper<wxComboBox>(sizeS1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
				m_comboTheme->AppendString(_("Light"));
				m_comboTheme->AppendString(_("Dark"));
				gridSizer->Add(themeText, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
				gridSizer->Add(m_comboTheme, 0);

				gridSizer->Add(new ThemedWrapper<wxStaticText>(sizeS1->GetStaticBox(), wxID_ANY, _("Language")));
				m_comboLanguage = new ThemedComboBoxWrapper<wxComboBox>(sizeS1->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
				m_comboLanguage->AppendString(_("System Language"));
				auto locales = wxTranslations::Get()->GetAvailableTranslations(wxT("UmaUmaChecker"));
				for (auto& locale : locales) {
					const wxLanguageInfo* info = wxLocale::FindLanguageInfo(locale);
					if (info) m_comboLanguage->AppendString(info->DescriptionNative);
				}
				gridSizer->Add(m_comboLanguage, 0);

				sizeS1->Add(gridSizer, 1, wxLEFT | wxBOTTOM, 5);
			}

			// 選択肢表示チェック
			m_checkBoxHideOption = new ThemedWrapper<wxCheckBox>(sizeS1->GetStaticBox(), wxID_ANY, _("Hide \"no choise\" event"));
			sizeS1->Add(m_checkBoxHideOption, 0, wxLEFT | wxBOTTOM, 5);

			// ステータス表示
			m_checkBoxShowStatusBar = new ThemedWrapper<wxCheckBox>(sizeS1->GetStaticBox(), wxID_ANY, _("Enable Status Bar"));
			sizeS1->Add(m_checkBoxShowStatusBar, 0, wxLEFT | wxBOTTOM, 5);

			// 更新確認
			m_checkBoxCheckUpdate = new ThemedWrapper<wxCheckBox>(sizeS1->GetStaticBox(), wxID_ANY, _("Notice on startup if there is available update"));
			sizeS1->Add(m_checkBoxCheckUpdate, 0, wxLEFT | wxBOTTOM, 5);

			sizeParent->Add(sizeS1, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// OCR設定
		wxStaticBoxSizer* sizeSystem = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, _("OCR Setting")), wxVERTICAL);
		{
			wxFlexGridSizer* gridSizer = new wxFlexGridSizer(1, 2, 3, 0);
			{
				wxStaticText* Static = new ThemedWrapper<wxStaticText>(sizeSystem->GetStaticBox(), wxID_ANY, _("Number of OCR"));
				gridSizer->Add(Static, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				wxBoxSizer* s = new wxBoxSizer(wxHORIZONTAL);
				{
					m_comboOcrPoolSize = new ThemedComboBoxWrapper<wxComboBox>(sizeSystem->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
					for (int i = 1; i <= 10; i++) m_comboOcrPoolSize->AppendString(wxString::Format(wxT("%d"), i));
					s->Add(m_comboOcrPoolSize, 0);

					s->Add(new ThemedWrapper<wxStaticText>(sizeSystem->GetStaticBox(), wxID_ANY, _("*Need to restart application.")), 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
				}
				gridSizer->Add(s);
				sizeSystem->Add(gridSizer, 1, wxALL, 5);
				sizeSystem->Add(new ThemedWrapper<wxStaticText>(sizeSystem->GetStaticBox(), wxID_ANY, _("Increase the value then CPU usage and Memory increase.\nDecrease the value then CPU usage and Memory decrease but be slowly recognize event.")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
			}

			wxBoxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			{
				wxStaticText* Static = new ThemedWrapper<wxStaticText>(sizeSystem->GetStaticBox(), wxID_ANY, _("Capture Type"));
				s2->Add(Static, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				m_comboCaptureMode = new ThemedComboBoxWrapper<wxComboBox>(sizeSystem->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
				m_comboCaptureMode->AppendString(_("BitBlt"));
				if (winrt_capture_is_supported()) m_comboCaptureMode->AppendString(_("Windows 10 (above 1903)"));
				s2->Add(m_comboCaptureMode);

				sizeSystem->Add(s2, 1, wxALL, 5);
			}

			sizeParent->Add(sizeSystem, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// スクリーンショット
		wxStaticBoxSizer* sizeS2 = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, _("Screenshot")), wxVERTICAL);
		{
			wxFlexGridSizer* fgSize = new wxFlexGridSizer(2, 2, 3, 5);
			fgSize->AddGrowableCol(1);
			{
				m_staticTextScreenShotPath = new ThemedWrapper<wxStaticText>(sizeS2->GetStaticBox(), wxID_ANY, _("Location"));
				fgSize->Add(m_staticTextScreenShotPath, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				wxBoxSizer* sspathSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					m_textCtrlScreenShotPath = new ThemedEditWrapper<wxTextCtrl>(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(280, -1)));
					sspathSizer->Add(m_textCtrlScreenShotPath, 1);

					m_buttonBrowse = new ThemedButtonWrapper<wxButton>(sizeS2->GetStaticBox(), wxID_ANY, _("Browse"));
					sspathSizer->Add(m_buttonBrowse);
				}

				fgSize->Add(sspathSizer, 0, wxEXPAND);
				fgSize->Add(new ThemedWrapper<wxStaticText>(sizeS2->GetStaticBox(), wxID_ANY, _("File Type")), 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

				m_comboFileType = new ThemedComboBoxWrapper<wxComboBox>(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_READONLY);
				m_comboFileType->Append(wxT("PNG"));
				m_comboFileType->Append(wxT("JPEG"));
				m_comboFileType->SetSelection(0);
				fgSize->Add(m_comboFileType, 0);
			}

			sizeS2->Add(fgSize, 1, wxEXPAND | wxALL, 5);
			sizeParent->Add(sizeS2, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);
		}

		// 高度な機能
		wxStaticBoxSizer* sizeS3 = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, _("Debug Mode")), wxVERTICAL);
		{
			m_checkDebugEnable = new ThemedWrapper<wxCheckBox>(sizeS3->GetStaticBox(), wxID_ANY, _("Write debug log in output.log"));
			m_checkSaveScreenShot = new ThemedWrapper<wxCheckBox>(sizeS3->GetStaticBox(), wxID_ANY, _("Capture screen if event is not recognized"));

			sizeS3->Add(m_checkDebugEnable, 0, wxALL, 5);
			sizeS3->Add(m_checkSaveScreenShot, 0, wxALL, 5);

			sizeParent->Add(sizeS3, 0, wxEXPAND | wxALL, 5);
		}

		// ダイアログボタン
		m_dialogButtonSizer = new wxStdDialogButtonSizer();
		wxButton* dbSizerOk = new ThemedButtonWrapper<wxButton>(this, wxID_OK);
		m_dialogButtonSizer->SetAffirmativeButton(dbSizerOk);
		wxButton* dbSizerCancel = new ThemedButtonWrapper<wxButton>(this, wxID_CANCEL, _("Cancel"));
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
	m_comboCaptureMode->SetSelection((int)config->CaptureMode);
}

void SettingDialog::OnClickUpdate(wxCommandEvent& event)
{
	if (UpdateManager::GetInstance().UpdateEvents()) {
		bUpdated = true;
		wxMessageBox(_("Update completed."), _("UmaUmaChecker"), wxICON_INFORMATION);
		return;
	}

	bUpdated = false;
	wxMessageBox(_("No available update."), _("UmaUmaChecker"), wxICON_INFORMATION);
}

void SettingDialog::OnClickBrowse(wxCommandEvent& event)
{
	wxDirDialog* dlg = new wxDirDialog(this, _("Select screenshot save location"), wxT(""), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
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
	config->CaptureMode = (CaptureMode)m_comboCaptureMode->GetSelection();

	this->GetParent()->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));
	this->EndModal(1);
}

void SettingDialog::OnClickFontSelect(wxCommandEvent& event)
{
}
