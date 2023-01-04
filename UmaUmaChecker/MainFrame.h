#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/tglbtn.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/statbox.h>
#include <wx/frame.h>

#include "wxTextPopupCtrl.h"
#include "wxUmaTextCtrl.h"
#include "wxComboBoxPopup.h"
#include "ThemeWrapper.h"

#include "PreviewFrame.h"
#include "DebugFrame.h"

#include "Uma.h"
#include "version.h"

class MainFrame : public ThemedWindowWrapper<wxFrame>
{
public:
	MainFrame(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxSYSTEM_MENU);
	virtual ~MainFrame();

private:
	void Init();
	bool LoadSkills();

	void OnClickStart(wxCommandEvent& event);
	void OnClickScreenShot(wxCommandEvent& event);
	void OnRightClickScreenShot(wxMouseEvent& event);
	void OnClickPreview(wxCommandEvent& event);
	void OnClickSetting(wxCommandEvent& event);
	void OnSelectedUma(wxCommandEvent& event);
	void OnUmaThreadEvent(wxThreadEvent& event);
	void OnEnterControl(wxMouseEvent& event);
	void OnLeaveControl(wxMouseEvent& event);
	void OnClickAbout(wxCommandEvent& event);
	void OnPreviewDragFile(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	// コンボボックスのオートコンプリート用
	void OnComboTextUpdate(wxCommandEvent& event);
	void OnSelectedListBoxItem(wxCommandEvent& event);
	void OnComboKeyDown(wxKeyEvent& event);
	// DPI用
	void OnDPIChanged(wxDPIChangedEvent& event);

	void ChangeEventOptions(EventSource* event);
	std::wstring GetSkillDescFromOption(const std::wstring& option);

	void ChangeTheme();

public:
	static void SetFontAllChildren(wxWindow* parent, const wxFont& font);

private:
	const int EventOptionCount = 5;

private:
	wxToggleButton* m_toggleBtnStart;
	wxButton* m_buttonScreenshot;
	wxButton* m_buttonPreview;
	wxButton* m_buttonSetting;
	wxButton* m_buttonAbout;
	wxStaticText* m_staticTextCharaName;
	wxComboBox* m_comboBoxUma;
	wxStaticText* m_staticTextEventName;
	wxTextCtrl* m_textCtrlEventSource;
	std::vector<wxTextCtrl*> m_textCtrlEventTitles;
	std::vector<wxUmaTextCtrl*> m_textCtrlEventOptions;
	PreviewFrame* m_PreviewWindow;
	wxTimer timer;
	wxStatusBar* m_statusBar;
	DebugFrame* m_DebugFrame = NULL;
	wxComboBoxPopup* m_comboPopup = NULL;

	Uma* umaMgr;
	std::unordered_map<std::wstring, std::wstring> SkillMap; // スキル名 -> 説明
};
