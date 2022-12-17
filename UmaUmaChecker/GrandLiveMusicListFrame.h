#pragma once

#include <wx/frame.h>
#include <wx/treelist.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <unordered_map>

class GrandLiveMusicListFrame : public wxFrame
{
public:
	GrandLiveMusicListFrame(wxWindow* parent);
	virtual ~GrandLiveMusicListFrame();

private:
	void LoadMusicList();
	wxString GetCheckedInfo();

	void OnClickClear(wxCommandEvent& event);
	void OnChecked(wxCommandEvent& event);

private:
	wxButton* m_buttonUncheckAll;
	wxTreeListCtrl* m_listCtrl;
	wxStaticText* m_text;
	std::unordered_map<int, std::wstring> TypeList;
};

