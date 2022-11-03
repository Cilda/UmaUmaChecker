#include "DebugFrame.h"

#include <wx/sizer.h>
#include <wx/button.h>

DebugFrame::DebugFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, wxT("デバッグウィンドウ"))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_comboOcrSource = new wxChoice(this, wxID_ANY);
	m_comboOcrSource->AppendString(wxT("イベントタイトル"));
	m_comboOcrSource->AppendString(wxT("選択肢"));
	m_comboOcrSource->SetSelection(0);

	sizer->Add(m_comboOcrSource, 0, wxEXPAND | wxALL, 5);

	m_textCtrlOcrResult = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	sizer->Add(m_textCtrlOcrResult, 1, wxEXPAND | wxALL, 5);

	m_buttonOcr = new wxButton(this, wxID_ANY, wxT("OCR"));
	sizer->Add(m_buttonOcr, 0, wxALL, 5);

	this->SetSizer(sizer);
	this->Layout();
	this->Centre(wxBOTH);

	m_buttonOcr->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebugFrame::OnClickOcr, this);
}

DebugFrame::~DebugFrame()
{
}

void DebugFrame::OnClickOcr(wxCommandEvent& event)
{
}

