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

#include "PreviewFrame.h"

#include "Uma.h"
#include "version.h"

class MainFrame : public wxFrame
{
public:
	MainFrame(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = app_title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(467, 500), long style = wxCAPTION | wxCLOSE_BOX | wxICONIZE | wxMINIMIZE_BOX | wxSYSTEM_MENU | wxTAB_TRAVERSAL);
	~MainFrame();

	void Init();
	void UnInit();

private:
	void OnClose(wxCloseEvent& event);
	void OnClickStart(wxCommandEvent& event);
	void OnClickScreenShot(wxCommandEvent& event);
	void OnClickPreview(wxCommandEvent& event);
	void OnClickSetting(wxCommandEvent& event);
	void OnSelectedUma(wxCommandEvent& event);
	void OnChangeUmaEvent(wxThreadEvent& event);
	void OnEnterControl(wxMouseEvent& event);
	void OnLeaveControl(wxMouseEvent& event);
	void OnClickAbout(wxCommandEvent& event);
	void OnPreviewDragFile(wxCommandEvent& event);

	void ChangeEventOptions(EventSource* event);
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

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
	std::vector<wxTextCtrl*> m_textCtrlEventOptions;

	PreviewFrame* m_PreviewWindow;

	Uma* umaMgr;
};
