#include "ThemeManager.h"

#include "StdRenderer.h"

#include <wx/app.h>


BaseRenderer* ThemeManager::render = nullptr;

std::list<FunctionRecord*> ThemeChangedNotify::clients;

ThemeManager::~ThemeManager()
{
    if (render) {
        delete render;
        render = nullptr;
    }
}

BaseRenderer* ThemeManager::GetInstance()
{
    if (!render) {
        render = new StdRenderer;
        render->OnInit();
    }

    return render;
}

void ThemeManager::SetTheme(const wxClassInfo* ClassInfo)
{
    if (render) {
        render->OnRelease();
        delete render;
    }

    render = (BaseRenderer*)ClassInfo->CreateObject();
    render->OnInit();

    NotifyAllControl(wxTheApp->GetTopWindow());
}

void ThemeManager::NotifyAllControl(wxWindow* parent)
{
    for (auto record : ThemeChangedNotify::clients) {
        record->method();
    }
}
