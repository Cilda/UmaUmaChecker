#include "BaseRenderer.h"

#include <wx/msw/uxtheme.h>


wxIMPLEMENT_ABSTRACT_CLASS(BaseRenderer, wxObject)

void BaseRenderer::AllowDarkMode(WXHWND hWnd, bool bAllow)
{
	/*
	using fnAllowDarkModeForWindow = bool(WINAPI*)(HWND hWnd, bool allow);
	HMODULE hModule = LoadLibraryEx(TEXT("uxtheme"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hModule) {
		fnAllowDarkModeForWindow AllowDarkModeForWindow = (fnAllowDarkModeForWindow)GetProcAddress(hModule, MAKEINTRESOURCEA(133));
		AllowDarkModeForWindow(hWnd, bAllow);
		FreeLibrary(hModule);
	}
	*/
}
