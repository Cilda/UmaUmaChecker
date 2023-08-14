#pragma once

#include <wx/popupwin.h>
#include <wx/listbox.h>

wxDECLARE_EVENT(wxEVT_HIDE, wxCommandEvent);

class wxComboBoxPopup : public wxPopupTransientWindow
{
public:
	wxComboBoxPopup(wxWindow* parent);
	virtual ~wxComboBoxPopup();

	void AddString(const wxString& text);
	void ClearList();

private:
	void OnComboClick(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);

public:
	wxListBox* m_listBox;
};

