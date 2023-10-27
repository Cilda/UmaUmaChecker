#include "Languages.h"

#include <memory>

#include <wx/string.h>
#include <wx/intl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace Languages {
	static std::unique_ptr<wxLocale> locale;

	void InitializeLanguages()
	{
		SetLang(wxT("System"));

		wxFileName path(wxStandardPaths::Get().GetExecutablePath());
		path.AppendDir(wxT("Languages"));
		path.SetFullName(wxT(""));

		wxFileTranslationsLoader::AddCatalogLookupPathPrefix(path.GetFullPath());

		/*wxTranslations* trans = new wxTranslations();
		wxTranslations::Set(trans);*/

		//trans->AddCatalog(wxT("UmaUmaChecker"));
	}

	void UninitializeLanguages()
	{
		locale.reset();
	}

	void SetLang(const wxString& lang)
	{
		locale.reset();

		int langId = wxLANGUAGE_DEFAULT;

		if (lang != wxT("System")) {
			wxArrayString langs = GetAvailableLanguages();
			if (langs.Index(lang) != wxNOT_FOUND) {
				const wxLanguageInfo* info = wxLocale::FindLanguageInfo(lang);
				if (info) {
					langId = info->Language;
				}
			}
		}

		locale = std::make_unique<wxLocale>(langId);

		wxFileName path(wxStandardPaths::Get().GetExecutablePath());
		path.AppendDir(wxT("Languages"));
		path.SetFullName(wxT(""));

		locale->AddCatalogLookupPathPrefix(path.GetFullPath());
		locale->AddCatalog(wxT("UmaUmaChecker"));
	}

	wxArrayString GetAvailableLanguages()
	{
		wxTranslations trans;
		wxArrayString langs = trans.GetAvailableTranslations(wxT("UmaUmaChecker"));
		return langs;
	}
}
