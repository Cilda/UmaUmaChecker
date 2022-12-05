#include "UpdateManager.h"

#include <wx/app.h>
#include <wx/webrequest.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>

#include "version.h"
#include "Config.h"
#include "CheckUpdateDialog.h"

UpdateManager::UpdateManager()
{
	m_timer.Bind(wxEVT_TIMER, &UpdateManager::OnTimer, this);
}

void UpdateManager::GetUpdates(bool bHideDontShowCheck)
{
	auto request = wxWebSession::GetDefault().CreateRequest(this, wxT("https://github.com/Cilda/UmaUmaChecker/releases.atom"));
	if (!request.IsOk()) return;

	this->Bind(wxEVT_WEBREQUEST_STATE, [&](wxWebRequestEvent& event) {
		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				VersionInfo version = ParseXmlData(event.GetResponse().GetStream());
#ifndef _DEBUG
				if (!version.title.empty() && version.title != app_version)
#endif
				{
					CheckUpdateDialog dialog(nullptr, &version, bHideDontShowCheck);
					dialog.ShowModal();
				}
#ifndef _DEBUG
				else if (bHideDontShowCheck) {
					wxMessageBox(wxT("最新バージョンです。"), app_name);
				}
#endif


				break;
			}
		}

		if (event.GetState() != wxWebRequest::State_Active) {
			request = wxWebRequest();
		}
	});

	request.Start();

	while (request.IsOk()) {
		wxYield();
	}
}

void UpdateManager::OnTimer(wxTimerEvent& event)
{
	auto config = Config::GetInstance();

	if (config->EnableCheckUpdate) GetUpdates();
}

UpdateManager::VersionInfo UpdateManager::ParseXmlData(wxInputStream* stream)
{
	VersionInfo version;
	wxXmlDocument doc(*stream);

	auto root = doc.GetRoot();
	auto children = root->GetChildren();

	wxString title;
	wxString content;

	while (children) {
		if (children->GetName() == wxT("entry")) {
			auto node = children->GetChildren();
			while (node) {
				if (node->GetName() == wxT("title")) {
					version.title = node->GetNodeContent();
				}
				else if (node->GetName() == wxT("content")) {
					version.content = node->GetNodeContent();
				}
				else if (node->GetName() == wxT("link")) {
					version.url = node->GetAttribute(wxT("href"));
				}

				node = node->GetNext();
			}
			break;
		}
		children = children->GetNext();
	}

	return version;
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
