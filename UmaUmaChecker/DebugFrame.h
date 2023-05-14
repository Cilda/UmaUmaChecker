#pragma once

#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/slider.h>

class DebugFrame : public wxFrame
{
public:
	DebugFrame(wxWindow* parent);
	virtual ~DebugFrame();

private:
	void OnClickOcr(wxCommandEvent& event);
	void OnDropFile(wxDropFilesEvent& event);

private:
	wxChoice* m_comboOcrSource;
	wxTextCtrl* m_textCtrlOcrResult;
	wxButton* m_buttonOcr;
	wxSlider* m_slider;
};

