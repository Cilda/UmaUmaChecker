#include "CombineThread.h"

#include <mutex>

#include "Images/Combine/SkillImageCombiner.h"
#include "Config/Config.h"
#include "Utils/utility.h"


wxDEFINE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMPLETED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMBINE_STARTED, wxThreadEvent);
wxDEFINE_EVENT(wxEVT_COMMAND_COMBINETHREAD_COMBINE_WAIT, wxThreadEvent);

CombineThread::CombineThread(wxWindow* parent) : wxThread(wxTHREAD_JOINABLE), parent(parent)
{
}

CombineThread::~CombineThread()
{
}

void* CombineThread::Entry()
{
	SkillImageCombiner combiner;
	std::once_flag once, once2;

	while (!TestDestroy()) {
		if (!combiner.Process(false)) break;

		if (!combiner.IsScanStarted()) {
			std::call_once(once, [&] {
				wxQueueEvent(parent, wxThreadEvent(wxEVT_COMMAND_COMBINETHREAD_COMBINE_WAIT).Clone());
			});
		}
		else {
			std::call_once(once2, [&] {
				wxQueueEvent(parent, wxThreadEvent(wxEVT_COMMAND_COMBINETHREAD_COMBINE_STARTED).Clone());
			});
		}
	}

	if (combiner.GetError().empty()) {
		if (TestDestroy()) {
			combiner.Process(true);
		}

		if (combiner.HasImages()) {
			Config* config = Config::GetInstance();
			std::filesystem::path basePath = !config->ScreenshotSavePath.empty() ? config->ScreenshotSavePath : utility::GetExeDirectory();

			combiner.Save(basePath / L"Screenshots" / (L"combine_" + utility::GetDateTimeString() + config->GetImageExtension()));
		}
	}

	wxThreadEvent event(wxEVT_COMMAND_COMBINETHREAD_COMPLETED);
	event.SetString(combiner.GetError());

	wxQueueEvent(parent, event.Clone());

	return (wxThread::ExitCode)0;
}
