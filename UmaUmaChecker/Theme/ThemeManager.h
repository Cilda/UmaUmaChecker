#pragma once

#include "BaseRenderer.h"

class ThemeManager final
{
private:
	ThemeManager();
	~ThemeManager();

public:
	static BaseRenderer* GetInstance();
	static void SetTheme(const wxClassInfo* ClassInfo);

private:
	static void NotifyAllControl(wxWindow* parent);

private:
	static BaseRenderer* render;
};

struct FunctionRecord
{
	std::function<void()> method;
	void* obj;
};


class ThemeChangedNotify
{
public:
	ThemeChangedNotify() {}
	~ThemeChangedNotify()
	{
		clients.remove(record);
		delete record;
	}

	template<typename Class>
	void Subscribe(void(Class::*method)(void), Class* handler)
	{
		record = new FunctionRecord;
		record->method = [=]() { (handler->*method)(); };
		record->obj = handler;

		clients.push_back(record);
	}
private:
	FunctionRecord* record;

public:
	static std::list<FunctionRecord*> clients;
};

