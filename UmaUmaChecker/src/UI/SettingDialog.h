#pragma once

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/webrequest.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include "FontComboBox.h"

#include "UI/Theme/ThemeWrapper.h"

class Config;

class SettingDialog : public ThemedWindowWrapper<wxDialog>
{
public:
	SettingDialog(wxWindow* parent, Config* config);
	~SettingDialog();

	bool IsUpdated() const { return bUpdated; }

private:
	void OnInitDialog(wxInitDialogEvent& event);
	void OnClickUpdate(wxCommandEvent& event);
	void OnClickBrowse(wxCommandEvent& event);
	void OnClickOkButton(wxCommandEvent& event);
	void OnClickFontSelect(wxCommandEvent& event);
	void OnSelectedLanguage(wxCommandEvent& event);

private:
	wxStaticText* m_staticTextUpdate;
	wxStaticText* m_staticTextScreenShotPath;
	wxButton* m_buttonUpdate;
	wxComboBox* m_comboFileType;
	wxTextCtrl* m_textCtrlScreenShotPath;
	wxButton* m_buttonBrowse;
	wxStdDialogButtonSizer* m_dialogButtonSizer;
	wxCheckBox* m_checkDebugEnable;
	wxCheckBox* m_checkSaveScreenShot;
	wxStaticText* m_staticTextFontSelect;
	wxFontPickerCtrl* m_fontPickerCtrl;
	wxCheckBox* m_checkBoxHideOption;
	wxCheckBox* m_checkBoxShowStatusBar;
	wxCheckBox* m_checkBoxCheckUpdate;
	wxStaticText* m_staticTextMaxLine;
	wxSpinCtrl* m_spinCtrlMaxLine;
	wxComboBox* m_comboOcrPoolSize;
	wxComboBox* m_comboTheme;
	wxComboBox* m_comboLanguage;
	FontComboBox* m_comboFontList;
	wxComboBox* m_comboFontSizeList;
	wxComboBox* m_comboCaptureMode;
	wxCheckBox* m_checkBoxEnableAutoStart;

	Config* config;
	std::vector<wxWebRequest> requests;
	bool bUpdated;
	int UpdatedCount;
};

