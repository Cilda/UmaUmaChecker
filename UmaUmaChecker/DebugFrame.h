#pragma once

#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>

class DebugFrame : public wxFrame
{
public:
	DebugFrame(wxWindow* parent);
	virtual ~DebugFrame();

private:
	void OnClickOcr(wxCommandEvent& event);

private:
	wxChoice* m_comboOcrSource;
	wxTextCtrl* m_textCtrlOcrResult;
	wxButton* m_buttonOcr;
};

