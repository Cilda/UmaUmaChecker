#include "UpdateManager.h"

#include <wx/app.h>
#include <wx/webrequest.h>

#include "CheckUpdateDialog.h"

UpdateManager::UpdateManager()
{
	m_timer.Bind(wxEVT_TIMER, &UpdateManager::OnTimer, this);
}

void UpdateManager::GetUpdates()
{
	auto request = wxWebSession::GetDefault().CreateRequest(this, wxT(""));
	if (!request.IsOk()) return;

	this->Bind(wxEVT_WEBREQUEST_STATE, [request](wxWebRequestEvent& event) {
		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				CheckUpdateDialog dialog(nullptr);
				dialog.ShowModal();
				break;
			}
		}
	});

	request.Start();
}

void UpdateManager::OnTimer(wxTimerEvent& event)
{
	GetUpdates();
}

UpdateManager& UpdateManager::GetInstance()
{
	static UpdateManager updateManager;

	return updateManager;
}

void UpdateManager::Start()
{
	auto& instance = GetInstance();

	instance.m_timer.StartOnce(1);
}
