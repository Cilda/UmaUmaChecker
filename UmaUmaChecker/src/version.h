#pragma once

#include <wx/string.h>

#ifdef _DEBUG
#define DEBUG_STRING wxT(" Debug")
#else
#define DEBUG_STRING wxT("")
#endif

static const wxString app_name = _("UmaUmaChecker");
static const wxString app_version = wxT("v1.6.2");
static const wxString app_copyright = wxT("Copyright © 2022-2023 Cilda.");
static const wxString app_title = app_name + wxT(" ") + app_version + DEBUG_STRING;