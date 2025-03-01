#include "AutocompleteThreadHelper.h"

AutocompleteThreadHelper::AutocompleteThreadHelper() : wxThreadHelper(wxTHREAD_JOINABLE)
{
}

AutocompleteThreadHelper::~AutocompleteThreadHelper()
{
}

void AutocompleteThreadHelper::StartAutocomplete()
{
	if (CreateThread(m_kind) != wxTHREAD_NO_ERROR) {
		return;
	}

	if (GetThread()->Run() != wxTHREAD_NO_ERROR) {
		return;
	}
}

wxThread::ExitCode AutocompleteThreadHelper::Entry()
{
	while (!GetThread()->TestDestroy()) {

	}

	return (wxThread::ExitCode)0;
}
