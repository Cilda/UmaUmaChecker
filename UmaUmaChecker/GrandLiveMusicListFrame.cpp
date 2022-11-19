#include "GrandLiveMusicListFrame.h"

#include <wx/sizer.h>

#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "utility.h"

using json = nlohmann::json;

GrandLiveMusicListFrame::GrandLiveMusicListFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, wxT("グランドライブ楽曲リスト"))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColor(255, 255, 255));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	m_buttonUncheckAll = new wxButton(this, wxID_ANY, wxT("チェックをすべて解除"));
	buttonSizer->Add(m_buttonUncheckAll);
	sizer->Add(buttonSizer, 0, wxALL, 5);

	m_listCtrl = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTL_CHECKBOX);
	m_listCtrl->AppendColumn(wxT("楽曲名"));
	m_listCtrl->AppendColumn(wxT("ボーナス"));
	m_listCtrl->AppendColumn(wxT("楽曲ボーナス"));
	m_listCtrl->AppendColumn(wxT("パフォーマンス"));
	sizer->Add(m_listCtrl, 1, wxEXPAND, 0);

	this->SetSizer(sizer);
	this->Layout();
	this->Centre(wxBOTH);

	LoadMusicList();
}

GrandLiveMusicListFrame::~GrandLiveMusicListFrame()
{
}

void GrandLiveMusicListFrame::LoadMusicList()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Scenario\\GrandLive.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			m_listCtrl->DeleteAllItems();

			json music = json::parse(text.str());

			for (auto info : music["MusicList"].items()) {
				int BonusType = info.value()["BonusType"].get<int>();
				int MasterBonusType = info.value()["MasterBonusType"].get<int>();
				int BonusValue = info.value()["BonusValue"].get<int>();
				int MasterBonusValue = info.value()["MasterBonusValue"].get<int>();

				std::wstring bonusDesc = utility::from_u8string(music["BonusType"][BonusType].get<std::string>());
				std::wstring masterBonusDesc = utility::from_u8string(music["BonusType"][MasterBonusType].get<std::string>());

				wxTreeListItem item = m_listCtrl->AppendItem(m_listCtrl->GetRootItem(), utility::from_u8string(info.value()["Name"].get<std::string>()));
				m_listCtrl->SetItemText(item, 1, wxString::Format(wxT("%s+%d%c"), bonusDesc, BonusValue, BonusType == 13 ? '%' : '\0'));
				m_listCtrl->SetItemText(item, 2, wxString::Format(wxT("%s+%d%c"), masterBonusDesc, MasterBonusValue, MasterBonusType == 13 ? '%' : '\0'));
			}
		}
		catch (json::exception& ex) {
			return;
		}
	}
}
