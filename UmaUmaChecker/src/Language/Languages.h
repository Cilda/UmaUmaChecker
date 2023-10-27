#pragma once

class wxString;
class wxArrayString;

namespace Languages {
	void InitializeLanguages();
	void UninitializeLanguages();
	void SetLang(const wxString& lang);
	wxArrayString GetAvailableLanguages();
};