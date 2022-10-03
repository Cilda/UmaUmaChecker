#include "wxTextPopupCtrl.h"

#include <Windows.h>
#include <wx/sizer.h>

wxTextPopupCtrl::wxTextPopupCtrl(wxWindow* parent, wxPoint pos, wxSize size) : wxFrame(parent, wxID_ANY, wxT(""), pos, size, wxPOPUP_WINDOW)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	SetWindowLongPtr(this->GetHWND(), GWL_STYLE, WS_POPUP | WS_BORDER);

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_textCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxPoint(0, 0), this->GetSize(), wxTE_READONLY | wxWANTS_CHARS | wxTE_RICH | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxBORDER_NONE);
	sizer->Add(m_textCtrl, 0, wxEXPAND, 0);

	this->SetSizer(sizer);
	this->Layout();

	m_textCtrl->Bind(wxEVT_LEAVE_WINDOW, &wxTextPopupCtrl::OnLeave, this);
	this->Bind(wxEVT_LEAVE_WINDOW, &wxTextPopupCtrl::OnLeave, this);
}

wxTextPopupCtrl::~wxTextPopupCtrl()
{
}

void wxTextPopupCtrl::OnLeave(wxMouseEvent& event)
{
	if (!this->IsShown()) return;

	Hide();
}
