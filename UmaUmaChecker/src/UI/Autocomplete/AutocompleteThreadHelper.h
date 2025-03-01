#pragma once

#include <wx/thread.h>

class AutocompleteThreadHelper : public wxThreadHelper
{
public:
	AutocompleteThreadHelper();
	~AutocompleteThreadHelper();



protected:
	void StartAutocomplete();

protected:
	virtual wxThread::ExitCode Entry() override;
};

