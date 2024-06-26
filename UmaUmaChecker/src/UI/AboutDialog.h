#pragma once

#include <wx/dialog.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/webrequest.h>

#include "Theme/ThemeWrapper.h"

class AboutDialog : public ThemedWindowWrapper<wxDialog>
{
public:
	AboutDialog(wxWindow* parent);
	~AboutDialog();

private:
	void OnClickUpdateCheck(wxCommandEvent& event);

	void CheckUpdate();

private:
	wxStaticBitmap* m_bitmap;
	wxButton* m_buttonOk;
	wxButton* m_buttonUpdateCheck;
};

