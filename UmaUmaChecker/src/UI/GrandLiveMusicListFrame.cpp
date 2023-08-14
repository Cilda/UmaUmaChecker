#include "GrandLiveMusicListFrame.h"

#include <wx/sizer.h>

#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Utils/utility.h"

using json = nlohmann::json;

class ItemData : public wxClientData {
public:
	int BonusType;
	int BonusValue;
	int MasterBonusType;
	int MasterBonusValue;
};

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
	m_listCtrl->AppendColumn(wxT("時期"));
	m_listCtrl->AppendColumn(wxT("Da"));
	m_listCtrl->AppendColumn(wxT("Pa"));
	m_listCtrl->AppendColumn(wxT("Vo"));
	m_listCtrl->AppendColumn(wxT("Vi"));
	m_listCtrl->AppendColumn(wxT("Me"));
	sizer->Add(m_listCtrl, 1, wxEXPAND, 0);

	m_text = new wxStaticText(this, wxID_ANY, wxEmptyString);
	sizer->Add(m_text, 0, wxALL, 0);

	this->SetSizer(sizer);
	this->Layout();
	this->Centre(wxBOTH);

	m_buttonUncheckAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GrandLiveMusicListFrame::OnClickClear, this);
	m_listCtrl->Bind(wxEVT_TREELIST_ITEM_CHECKED, &GrandLiveMusicListFrame::OnChecked, this);

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

			int typeIndex = 0;
			for (auto info : music["BonusType"]) {
				TypeList[typeIndex] = utility::from_u8string(info.get<std::string>());
				typeIndex++;
			}

			for (auto info : music["MusicList"].items()) {
				int BonusType = info.value()["BonusType"].get<int>();
				int MasterBonusType = info.value()["MasterBonusType"].get<int>();
				int BonusValue = info.value()["BonusValue"].get<int>();
				int MasterBonusValue = info.value()["MasterBonusValue"].get<int>();

				std::wstring bonusDesc = utility::from_u8string(music["BonusType"][BonusType].get<std::string>());
				std::wstring masterBonusDesc = utility::from_u8string(music["BonusType"][MasterBonusType].get<std::string>());

				ItemData* minfo = new ItemData();
				minfo->BonusType = BonusType;
				minfo->BonusValue = BonusValue;
				minfo->MasterBonusType = MasterBonusType;
				minfo->MasterBonusValue = MasterBonusValue;

				wxTreeListItem item = m_listCtrl->AppendItem(m_listCtrl->GetRootItem(), utility::from_u8string(info.value()["Name"].get<std::string>()));
				m_listCtrl->SetItemText(item, 1, wxString::Format(wxT("%s+%d%c"), bonusDesc, BonusValue, BonusType == 13 ? '%' : '\0'));
				m_listCtrl->SetItemText(item, 2, wxString::Format(wxT("%s+%d%c"), masterBonusDesc, MasterBonusValue, MasterBonusType == 13 ? '%' : '\0'));
				m_listCtrl->SetItemText(item, 3, utility::from_u8string(info.value()["Year"].get<std::string>()));

				m_listCtrl->SetItemData(item, minfo);

				for (int i = 0; i < 5; i++) m_listCtrl->SetItemText(item, 4 + i, wxString::Format(wxT("%d"), info.value()["ReqPerf"][i].get<int>()));
			}
		}
		catch (json::exception& ex) {
			return;
		}
	}
}

wxString GrandLiveMusicListFrame::GetCheckedInfo()
{
	std::unordered_map<int, int> state;

	for (wxTreeListItem item = m_listCtrl->GetFirstItem(); item.IsOk(); item = m_listCtrl->GetNextItem(item)) {
		if (m_listCtrl->GetCheckedState(item) == wxCHK_CHECKED) {
			ItemData* data = (ItemData*)m_listCtrl->GetItemData(item);

			if (state.find(data->BonusType) == state.end()) state[data->BonusType] = 0;
			state[data->BonusType] += data->BonusValue;

			if (state.find(data->MasterBonusType) == state.end()) state[data->MasterBonusType] = 0;
			state[data->MasterBonusType] += data->MasterBonusValue;
		}
	}

	wxString ret;

	for (auto& itr : state) {
		if (itr.first >= 6 && itr.first <= 11) continue;

		ret += wxString::Format(wxT("%s+%d%c"), TypeList[itr.first], itr.second, itr.first == 13 ? '%' : '\0') + wxT("\n");
	}

	return ret;
}

void GrandLiveMusicListFrame::OnClickClear(wxCommandEvent& event)
{
	for (wxTreeListItem item = m_listCtrl->GetFirstItem(); item.IsOk(); item = m_listCtrl->GetNextItem(item)) {
		if (m_listCtrl->GetCheckedState(item) == wxCHK_CHECKED) {
			m_listCtrl->UncheckItem(item);
		}
	}

	wxString value = GetCheckedInfo();

	m_text->SetLabel(value);
	this->Layout();
}

void GrandLiveMusicListFrame::OnChecked(wxCommandEvent& event)
{
	wxString value = GetCheckedInfo();

	m_text->SetLabel(value);
	this->Layout();
}
