#include "UpdateManager.h"

#include <wx/app.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/wfstream.h>

#include "version.h"
#include "Config/Config.h"
#include "Utils/utility.h"

#include "Log/Log.h"

#include "UI/CheckUpdateDialog.h"

UpdateManager::UpdateManager()
{
	m_timer.Bind(wxEVT_TIMER, &UpdateManager::OnTimer, this);
}

void UpdateManager::GetUpdates(wxWindow* parent, bool bHideDontShowCheck)
{
	auto request = wxWebSession::GetDefault().CreateRequest(this, wxT("https://github.com/Cilda/UmaUmaChecker/releases.atom"));
	if (!request.IsOk()) return;

	this->Bind(wxEVT_WEBREQUEST_STATE, [&](wxWebRequestEvent& event) {
		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				VersionInfo version = ParseXmlData(event.GetResponse().GetStream());
#ifndef _DEBUG
				if (!version.title.empty() && CheckVersion(version.title))
#endif
				{
					CheckUpdateDialog dialog(parent, &version, bHideDontShowCheck);
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

bool UpdateManager::UpdateEvents()
{
	std::vector<wxString> urls{
		wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Chara.json"),
		wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Events.json"),
		wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/ReplaceText.json"),
		wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/ScenarioEvents.json"),
		wxT("https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Skills.json"),
	};

	int UpdatedCount = 0;

	for (auto& url : urls) {
		if (UpdateFile(url)) UpdatedCount++;
	}

	return UpdatedCount > 0;
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

bool UpdateManager::CheckVersion(wxString version)
{
	return ConvertVersion(app_version) < ConvertVersion(version);
}

int UpdateManager::ConvertVersion(wxString version)
{
	wxRegEx ex("v(\\d+)\\.(\\d+)\\.(\\d+)");
	if (ex.Matches(version)) {
		int high, mid, low;

		ex.GetMatch(version, 1).ToInt(&high);
		ex.GetMatch(version, 2).ToInt(&mid);
		ex.GetMatch(version, 3).ToInt(&low);

		return high * 100 + mid * 10 + low;
	}

	return 0;
}

bool UpdateManager::UpdateFile(const wxString& url)
{
	wxWebRequest request = wxWebSession::GetDefault().CreateRequest(this, url);

	if (!request.IsOk()) {
		return false;
	}

	bool bUpdated = false;

	this->Bind(wxEVT_WEBREQUEST_STATE, [&](wxWebRequestEvent& event) {
		bool IsActive = false;

		switch (event.GetState()) {
			case wxWebRequest::State_Completed: {
				const wxWebResponse& response = event.GetResponse();
				if (response.GetStatus() == 200) {
					auto stream = response.GetStream();
					wxString path = utility::GetExeDirectory() + L"\\Library\\" + response.GetSuggestedFileName();
					int size = 0;

					if (wxFile::Exists(path)) {
						wxFile file(path);
						if (file.IsOpened()) {
							size = file.SeekEnd();
							file.Close();
						}
					}

					if (size != stream->GetSize()) {
						LOG_INFO << L"UpdateFile: " << response.GetSuggestedFileName() << L" 更新あり";

						wxFileOutputStream output(path);

						if (output.IsOk()) {
							output.Write(*stream);
							output.Close();
							bUpdated = true;
						}
					}
				}
				break;
			}
			case wxWebRequest::State_Active:
				IsActive = true;
				break;
		}

		if (!IsActive) {
			request = wxWebRequest();
		}
	});

	request.Start();

	while (request.IsOk()) {
		wxYield();
	}

	return bUpdated;
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
