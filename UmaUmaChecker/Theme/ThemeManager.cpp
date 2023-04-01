#include "ThemeManager.h"

#include "StdRenderer.h"

#include <wx/app.h>


std::unique_ptr<BaseRenderer> ThemeManager::render;

std::list<FunctionRecord*> ThemeChangedNotify::clients;

ThemeManager::~ThemeManager()
{
}

BaseRenderer* ThemeManager::GetInstance()
{
    if (!render) {
        render = std::unique_ptr<BaseRenderer>(new StdRenderer);
        render->OnInit();
    }

    return render.get();
}

void ThemeManager::SetTheme(const wxClassInfo* ClassInfo)
{
    if (render) {
        render->OnRelease();
        render.reset();
    }

    render = std::unique_ptr<BaseRenderer>((BaseRenderer*)ClassInfo->CreateObject());
    render->OnInit();

    NotifyAllControl(wxTheApp->GetTopWindow());
}

void ThemeManager::NotifyAllControl(wxWindow* parent)
{
    for (auto record : ThemeChangedNotify::clients) {
        record->method();
    }
}
