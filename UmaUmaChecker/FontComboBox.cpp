#include "FontComboBox.h"

#include <wx/dcclient.h>
#include <wx/fontenum.h>

FontComboBox::FontComboBox(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, int n, const wxString choices[], long style, const wxValidator& validator, const wxString& name)
	: wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style, validator, name)
{
	wxFontEnumerator enumrator;
	auto fonts = enumrator.GetFacenames(wxFONTENCODING_CP932);
	for (auto& font : fonts) {
		if (font[0] != '@') this->AppendString(font);
	}
	this->InvalidateBestSize();
}

void FontComboBox::OnDrawBackground(wxDC& dc, const wxRect& rect, int item, int flags) const
{
	wxOwnerDrawnComboBox::OnDrawBackground(dc, rect, item, flags);
}

void FontComboBox::OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const
{
	wxString name = this->GetString(item);
	auto font = dc.GetFont();

	font.SetFaceName(name);
	dc.SetFont(font);

	dc.DrawText(name, rect.x + 3, rect.y + (rect.height - dc.GetCharHeight()) / 2);
}

wxCoord FontComboBox::OnMeasureItem(size_t item) const
{
	return FromDIP(this->GetCharHeight());
}

wxCoord FontComboBox::OnMeasureItemWidth(size_t item) const
{
	int w, h;
	wxString value = GetString(item);
	wxFont font = GetFont();

	font.SetFaceName(value);
	GetTextExtent(value, &w, &h, NULL, NULL, &font);

	return w + 4;
}
