#pragma once

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

class Config;

class SettingDialog : public wxDialog
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

	bool UpdateLibrary();
	bool UpdateFile(const std::wstring& url, const std::wstring& path);

private:
	wxStaticText* m_staticTextUpdate;
	wxStaticText* m_staticTextScreenShotPath;
	wxButton* m_buttonUpdate;
	wxTextCtrl* m_textCtrlScreenShotPath;
	wxButton* m_buttonBrowse;
	wxStdDialogButtonSizer* m_dialogButtonSizer;
	wxCheckBox* m_checkDebugEnable;
	wxCheckBox* m_checkSaveScreenShot;

	Config* config;

	bool bUpdated;
};

