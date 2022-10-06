#include <winsock2.h>
#include "MainFrame.h"

#include <Windows.h>
#include <gdiplus.h>
#include <chrono>
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <regex>

#include "utility.h"

#include "SettingDialog.h"

MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style), umaMgr(new Uma(this))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer(wxHORIZONTAL);

	m_toggleBtnStart = new wxToggleButton(this, wxID_ANY, wxT("スタート"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer35->Add(m_toggleBtnStart, 0, wxALL, 5);

	m_buttonScreenshot
		= new wxButton(this, wxID_ANY, wxT("スクリーンショット"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer35->Add(m_buttonScreenshot
		, 0, wxALL, 5);

	m_buttonPreview = new wxButton(this, wxID_ANY, wxT("プレビュー表示"), wxDefaultPosition, wxDefaultSize, 0);
	m_buttonPreview->Enable(false);
	bSizer35->Add(m_buttonPreview, 0, wxALL, 5);

	m_buttonSetting = new wxButton(this, wxID_ANY, wxT("設定"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer35->Add(m_buttonSetting, 0, wxALL, 5);


	bSizer30->Add(bSizer35, 0, wxEXPAND | wxFIXED_MINSIZE | wxRIGHT | wxLEFT, 5);

	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizer5->SetFlexibleDirection(wxBOTH);
	fgSizer5->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_staticText6 = new wxStaticText(this, wxID_ANY, wxT("育成ウマ娘"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(-1);
	fgSizer5->Add(m_staticText6, 0, wxALL, 5);

	m_comboBoxUma = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	m_comboBoxUma->SetMinSize(wxSize(300, -1));

	fgSizer5->Add(m_comboBoxUma, 0, wxALL, 5);

	m_staticText10 = new wxStaticText(this, wxID_ANY, wxT("イベント名"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText10->Wrap(-1);
	fgSizer5->Add(m_staticText10, 0, wxALL, 5);

	m_textCtrlEventSource = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_READONLY);
	m_textCtrlEventSource->SetBackgroundColour(wxColour(255, 255, 255));

	fgSizer5->Add(m_textCtrlEventSource, 0, wxALL, 5);
	bSizer42->Add(fgSizer5, 0, wxEXPAND, 5);
	bSizer30->Add(bSizer42, 0, wxEXPAND | wxRIGHT | wxLEFT, 5);

	wxStaticBoxSizer* sbSizer3;
	sbSizer3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("選択肢")), wxVERTICAL);

	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer(wxHORIZONTAL);

	m_textCtrlEvent1 = new wxTextCtrl(sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent1->SetBackgroundColour(wxColour(200, 255, 150));
	m_textCtrlEvent1->SetMinSize(wxSize(180, -1));

	bSizer39->Add(m_textCtrlEvent1, 0, wxALL, 5);

	m_richText1 = new wxUmaTextCtrl(sbSizer3->GetStaticBox());
	bSizer39->Add(m_richText1, 1, wxALL | wxEXPAND, 5);

	sbSizer3->Add(bSizer39, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer(wxHORIZONTAL);

	m_textCtrlEvent2 = new wxTextCtrl(sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent2->SetBackgroundColour(wxColour(255, 240, 140));
	m_textCtrlEvent2->SetMinSize(wxSize(180, -1));

	bSizer41->Add(m_textCtrlEvent2, 0, wxALL, 5);

	m_richText2 = new wxUmaTextCtrl(sbSizer3->GetStaticBox());
	bSizer41->Add(m_richText2, 1, wxEXPAND | wxALL, 5);


	sbSizer3->Add(bSizer41, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer(wxHORIZONTAL);

	m_textCtrlEvent3 = new wxTextCtrl(sbSizer3->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent3->SetBackgroundColour(wxColour(255, 200, 200));
	m_textCtrlEvent3->SetMinSize(wxSize(180, -1));

	bSizer40->Add(m_textCtrlEvent3, 0, wxALL, 5);

	m_richText3 = new wxUmaTextCtrl(sbSizer3->GetStaticBox());
	bSizer40->Add(m_richText3, 1, wxEXPAND | wxALL, 5);

	sbSizer3->Add(bSizer40, 1, wxEXPAND, 5);
	bSizer30->Add(sbSizer3, 1, wxEXPAND | wxALL, 5);

	this->SetSizer(bSizer30);
	this->Layout();
	this->Centre(wxBOTH);

	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	m_toggleBtnStart->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &MainFrame::OnClickStart, this);
	m_buttonScreenshot->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickScreenShot, this);
	m_buttonPreview->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickPreview, this);
	m_buttonSetting->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickSetting, this);
	m_comboBoxUma->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainFrame::OnSelectedUma, this);
	m_richText1->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnEnterControl, this);
	m_richText1->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnLeaveControl, this);
	m_richText2->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnEnterControl, this);
	m_richText2->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnLeaveControl, this);
	m_richText3->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnEnterControl, this);
	m_richText3->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnLeaveControl, this);
	this->Bind(wxEVT_THREAD, &MainFrame::OnChangeUmaEvent, this);
}

MainFrame::~MainFrame()
{
	delete umaMgr;
}

void MainFrame::Init()
{
	umaMgr->Init();

	for (auto& chara : umaMgr->GetCharacters()) {
		m_comboBoxUma->Append(chara->Name);
	}
}

void MainFrame::UnInit()
{
	umaMgr->config.Save();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	umaMgr->config.Save();
	Destroy();
}

void MainFrame::OnClickStart(wxCommandEvent& event)
{
	if (!m_toggleBtnStart->GetValue()) {
		umaMgr->Stop();
		m_toggleBtnStart->SetLabelText(wxT("スタート"));
	}
	else {
		umaMgr->Start();
		m_toggleBtnStart->SetLabelText(wxT("ストップ"));
	}
}

void MainFrame::OnClickScreenShot(wxCommandEvent& event)
{
	Gdiplus::Bitmap* image = umaMgr->ScreenShot();
	if (image) {
		CLSID clsid;

		std::wstring directory = umaMgr->config.ScreenshotSavePath + L"\\";
		if (umaMgr->config.ScreenshotSavePath.empty()) {
			directory = utility::GetExeDirectory() + L"\\screenshots\\";
			CreateDirectoryW(directory.c_str(), NULL);
		}

		std::wstring savename = directory
			+ std::wstring(L"screenshot_")
			+ std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
			+ L".png";

		GetEncoderClsid(L"image/png", &clsid);
		image->Save(savename.c_str(), &clsid);
		delete image;
	}
	else {
		wxMessageBox(wxT("ウマ娘のウィンドウが見つかりません。"), wxT("ウマウマチェッカー"));

	}
}

void MainFrame::OnClickPreview(wxCommandEvent& event)
{
}

void MainFrame::OnClickSetting(wxCommandEvent& event)
{
	SettingDialog* frame = new SettingDialog(this, &umaMgr->config);
	if (frame->ShowModal() == 1) {

	}
	frame->Destroy();
}

void MainFrame::OnSelectedUma(wxCommandEvent& event)
{
	umaMgr->SetTrainingCharacter(m_comboBoxUma->GetValue().wc_str());
}

void MainFrame::OnChangeUmaEvent(wxThreadEvent& event)
{
	if (umaMgr->CurrentEvent) {
		m_textCtrlEventSource->SetValue(umaMgr->CurrentEvent->Name);
		wxTextCtrl* controls[3] = { m_textCtrlEvent1, m_textCtrlEvent2, m_textCtrlEvent3 };
		wxUmaTextCtrl* richCtrls[3] = { m_richText1, m_richText2, m_richText3 };
		
		for (int i = 0; i < 3; i++) {
			if (i < umaMgr->CurrentEvent->Options.size()) {
				controls[i]->SetValue(umaMgr->CurrentEvent->Options[i]->Title);
				richCtrls[i]->SetValue(umaMgr->CurrentEvent->Options[i]->Effect);
			}
			else {
				controls[i]->SetValue(wxT(""));
				richCtrls[i]->SetValue(wxT(""));
			}
		}
	}
}

void MainFrame::OnEnterControl(wxMouseEvent& event)
{
	wxTextCtrl* ctrl = (wxTextCtrl*)event.GetEventObject();
	wxSize clientSize = ctrl->GetClientSize();
	wxSize windowSize = ctrl->GetSize();
	wxString text = ctrl->GetValue();
	wxTextAttr attr = ctrl->GetDefaultStyle();
	wxClientDC dc(ctrl);
	dc.SetFont(attr.GetFont());
	wxSize size = dc.GetMultiLineTextExtent(text);

	int marginWidth = windowSize.x - clientSize.x;
	int marginHeight = windowSize.y - clientSize.y;

	if (size.x > clientSize.x || size.y > clientSize.y) {
		int width = size.x > clientSize.x ? size.x : clientSize.x;
		int height = size.y > clientSize.y ? size.y : clientSize.y;

		wxPoint pos = ctrl->ClientToScreen(wxPoint(-2, -2));
		wxTextPopupCtrl* m_textPopup = new wxTextPopupCtrl(this, wxSize(width + marginWidth, height + marginHeight));
		m_textPopup->Position(pos, wxSize(0, 0));
		m_textPopup->SetText(text);
		m_textPopup->Popup();
	}
}

void MainFrame::OnLeaveControl(wxMouseEvent& event)
{
}

int MainFrame::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return -1;
}