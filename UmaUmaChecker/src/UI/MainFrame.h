#pragma once

#include <thread>

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/tglbtn.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
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
#include "Autocomplete/AutocompleteWrapper.h"
#include "UI/Theme/ThemeWrapper.h"
#include "Recognizer/CombineImage.h"

#include "PreviewFrame.h"
#include "DebugFrame.h"


#include "Recognizer/Uma.h"
#include "version.h"

class MainFrame : public ThemedWindowWrapper<wxFrame>
{
public:
	MainFrame(wxWindow* parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxSYSTEM_MENU | wxRESIZE_BORDER);
	virtual ~MainFrame();

private:
	void Init();
	void AddToSystemMenu();

	void OnClose(wxCloseEvent& event);
	//void OnSize(wxSizeEvent& event);
	//void OnSizing(wxSizeEvent& event);
	void OnClickStart(wxCommandEvent& event);
	void OnClickScreenShot(wxCommandEvent& event);
	void OnRightClickScreenShot(wxMouseEvent& event);
	void OnClickCombine(wxCommandEvent& event);
	void OnClickPreview(wxCommandEvent& event);
	void OnClickSetting(wxCommandEvent& event);
	void OnSelectedUma(wxCommandEvent& event);
	void OnUmaThreadEvent(wxThreadEvent& event);
	void OnEnterControl(wxMouseEvent& event);
	void OnLeaveControl(wxMouseEvent& event);
	void OnClickAbout(wxCommandEvent& event);
	void OnPreviewDragFile(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnCombineTimer(wxTimerEvent& event);
	// コンボボックスのオートコンプリート用
	void OnComboTextUpdate(wxCommandEvent& event);
	void OnSelectedListBoxItem(wxCommandEvent& event);
	void OnComboKeyDown(wxKeyEvent& event);
	void OnAutocompleteEvent(wxCommandEvent& event);

	// DPI用
	void OnDPIChanged(wxDPIChangedEvent& event);

	bool MSWTranslateMessage(WXMSG* msg) override;

	void ChangeEventOptions(EventSource* event);
	std::wstring GetSkillDescFromOption(const std::wstring& option);

	void ChangeTheme();
	void SetTrainingCharaComboBox();

public:
	static void SetFontAllChildren(wxWindow* parent, const wxFont& font);

private:
	const int EventOptionCount = 6;

private:
	wxToggleButton* m_toggleBtnStart;
	wxBitmapButton* m_buttonScreenshot;
	wxBitmapButton* m_buttonCombine;
	wxButton* m_buttonPreview;
	wxButton* m_buttonSetting;
	wxButton* m_buttonAbout;
	wxStaticText* m_staticTextCharaName;
	wxComboBox* m_comboBoxUma;
	wxStaticText* m_staticTextEventName;
	AutocompleteWrapper<wxTextCtrl>* m_textCtrlEventSource;
	std::vector<wxTextCtrl*> m_textCtrlEventTitles;
	std::vector<wxUmaTextCtrl*> m_textCtrlEventOptions;
	PreviewFrame* m_PreviewWindow;
	wxTimer timer;
	wxStatusBar* m_statusBar;
	DebugFrame* m_DebugFrame = NULL;
	wxComboBoxPopup* m_comboPopup = NULL;

	Uma* umaMgr;

	wxTimer CombineTimer;
	CombineImage combine;
	std::thread thread;
};
