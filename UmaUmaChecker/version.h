#pragma once

#include <wx/string.h>

#ifdef _DEBUG
#define DEBUG_STRING wxT(" Debug")
#else
#define DEBUG_STRING
#endif

static const wxString app_name = wxT("ウマウマチェッカー");
static const wxString app_version = wxT("v1.3.6") DEBUG_STRING;
static const wxString app_copyright = wxT("Copyright (C) 2022 Cilda.");
static const wxString app_title = app_name + wxT(" ") + app_version;