#include "wxComboBoxPopup.h"

#include <wx/sizer.h>
#include <wx/panel.h>

#include "UI/Theme/ThemeWrapper.h"

wxDEFINE_EVENT(wxEVT_HIDE, wxCommandEvent);

wxComboBoxPopup::wxComboBoxPopup(wxWindow* parent) : wxPopupTransientWindow(parent, wxBORDER_NONE)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	m_listBox = new ThemedListBoxWrapper<wxListBox>(this, wxID_ANY);
	sizer->Add(m_listBox, 1, wxEXPAND, 0);
	this->SetSizer(sizer);

	//this->Fit();

	m_listBox->Bind(wxEVT_LEFT_DOWN, &wxComboBoxPopup::OnComboClick, this);
	m_listBox->Bind(wxEVT_MOTION, &wxComboBoxPopup::OnMouseMove, this);
}

wxComboBoxPopup::~wxComboBoxPopup()
{
}

void wxComboBoxPopup::AddString(const wxString& text)
{
	m_listBox->Insert(text, 0);
}

void wxComboBoxPopup::ClearList()
{
	m_listBox->Clear();
}

void wxComboBoxPopup::OnComboClick(wxMouseEvent& event)
{
	int index = m_listBox->HitTest(event.GetPosition());
	if (index != wxNOT_FOUND) {
		wxCommandEvent event(wxEVT_HIDE);
		m_listBox->SetSelection(index);
		event.SetString(m_listBox->GetString(index));
		ProcessWindowEvent(event);
		this->Dismiss();
	}
}

void wxComboBoxPopup::OnMouseMove(wxMouseEvent& event)
{
	int index = m_listBox->HitTest(event.GetPosition());
	if (index != wxNOT_FOUND) {
		if (index != m_listBox->GetSelection())
			m_listBox->SetSelection(index);
	}
}
