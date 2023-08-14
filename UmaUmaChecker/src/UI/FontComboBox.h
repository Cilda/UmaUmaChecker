#pragma once

#include <wx/combo.h>
#include <wx/odcombo.h>

class FontComboBox : public wxOwnerDrawnComboBox
{
public:
	FontComboBox(wxWindow* parent, wxWindowID id, const wxString& value = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = "comboBox");

protected:
	virtual void OnDrawBackground(wxDC& dc, const wxRect& rect, int item, int flags) const;
	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
	virtual wxCoord OnMeasureItem(size_t item) const;
	virtual wxCoord OnMeasureItemWidth(size_t item) const;
};

