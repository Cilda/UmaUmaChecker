#include "wxTextPopupCtrl.h"

#include <Windows.h>
#include <wx/sizer.h>


wxTextPopupCtrl::wxTextPopupCtrl(wxWindow* parent, const wxSize& size) : wxPopupTransientWindow(parent, wxBORDER_NONE | wxPU_CONTAINS_CONTROLS)
{
	m_panel = new wxWindow(this, wxID_ANY);
	m_panel->SetSize(size);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_textCtrl = new wxUmaTextCtrl(m_panel);
	m_textCtrl->SetSize(size);
	m_textCtrl->SetBackgroundColour(wxColour(235, 253, 250));
	sizer->Add(m_textCtrl, 1, wxEXPAND, 0);

	m_panel->SetSizer(sizer);

	this->SetClientSize(m_panel->GetSize());

	this->Bind(wxEVT_MOTION, &wxTextPopupCtrl::OnMouseMove, this);
	this->Bind(wxEVT_LEAVE_WINDOW, &wxTextPopupCtrl::OnMouseLeave, this);
	m_textCtrl->Bind(wxEVT_LEAVE_WINDOW, &wxTextPopupCtrl::OnCtrlLeave, this);
	m_textCtrl->Bind(wxEVT_MOTION, &wxTextPopupCtrl::OnMouseMove, this);
}

wxTextPopupCtrl::~wxTextPopupCtrl()
{
}

void wxTextPopupCtrl::Popup(wxWindow* focus)
{
	wxPopupTransientWindow::Popup(focus);
}

void wxTextPopupCtrl::Dismiss()
{
	wxPopupTransientWindow::Dismiss();
}

void wxTextPopupCtrl::SetText(const wxString& text)
{
	m_textCtrl->SetValue(text);
}

bool wxTextPopupCtrl::ProcessLeftDown(wxMouseEvent& event)
{
	return true;
}

void wxTextPopupCtrl::OnMouseMove(wxMouseEvent& event)
{
	if (!this->IsMouseInWindow())
		this->Dismiss();

	event.Skip();
}

void wxTextPopupCtrl::OnMouseLeave(wxMouseEvent& event)
{
	this->Dismiss();
}

void wxTextPopupCtrl::OnCtrlLeave(wxMouseEvent& event)
{
	this->Dismiss();
}

void wxTextPopupCtrl::OnDismiss()
{
	wxPopupTransientWindow::OnDismiss();
}
