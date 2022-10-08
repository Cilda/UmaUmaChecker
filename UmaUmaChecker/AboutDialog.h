#pragma once

#include <wx/dialog.h>
#include <wx/statbmp.h>
#include <wx/button.h>


class AboutDialog : public wxDialog
{
public:
	AboutDialog(wxWindow* parent);
	~AboutDialog();

private:
	wxStaticBitmap* m_bitmap;
	wxButton* m_buttonOk;
	wxButton* m_buttonLicense;
};

