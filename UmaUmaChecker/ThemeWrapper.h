#pragma once

#include <wx/window.h>

template<class WindowBase>
class ThemedWrapper final : public WindowBase
{
public:
	template<typename... Args>
	ThemedWrapper(Args&&... args) : WindowBase(std::forward<Args>(args)...)
	{

	}

};

