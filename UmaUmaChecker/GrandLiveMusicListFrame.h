#pragma once

#include <wx/frame.h>
#include <wx/treelist.h>
#include <wx/button.h>

class GrandLiveMusicListFrame : public wxFrame
{
public:
	GrandLiveMusicListFrame(wxWindow* parent);
	virtual ~GrandLiveMusicListFrame();

private:
	void LoadMusicList();

private:
	wxButton* m_buttonUncheckAll;
	wxTreeListCtrl* m_listCtrl;
};
