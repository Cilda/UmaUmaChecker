#pragma once

#include <wx/dialog.h>
#include "UpdateManager.h"

class CheckUpdateDialog : public wxDialog
{
public:
	CheckUpdateDialog(wxWindow* parent, UpdateManager::VersionInfo* version, bool bHideDontShowCheck);
	virtual ~CheckUpdateDialog();

private:
	void OnDontShow(wxCommandEvent& event);
	void OnClickUpdate(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxString url;
};

