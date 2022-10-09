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
#include "AboutDialog.h"

MainFrame::MainFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, id, title, pos, size, style), umaMgr(new Uma(this)), m_PreviewWindow(NULL)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* bSizerTop = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* bSizerButtons = new wxBoxSizer(wxHORIZONTAL);

	m_toggleBtnStart = new wxToggleButton(this, wxID_ANY, wxT("スタート"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_toggleBtnStart, 0, wxALL, 5);

	m_buttonScreenshot
		= new wxButton(this, wxID_ANY, wxT("スクリーンショット"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonScreenshot, 0, wxALL, 5);

	m_buttonPreview = new wxButton(this, wxID_ANY, wxT("プレビュー表示"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonPreview, 0, wxALL, 5);

	m_buttonSetting = new wxButton(this, wxID_ANY, wxT("設定"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonSetting, 0, wxALL, 5);

	m_buttonAbout = new wxButton(this, wxID_ANY, wxT("About"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonAbout, 0, wxALL, 5);

	bSizerTop->Add(bSizerButtons, 0, wxEXPAND | wxFIXED_MINSIZE | wxRIGHT | wxLEFT, 5);

	wxBoxSizer* bSizer42;
	bSizer42 = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* fgSizerInfo = new wxFlexGridSizer(2, 2, 0, 0);
	fgSizerInfo->SetFlexibleDirection(wxBOTH);
	fgSizerInfo->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	m_staticTextCharaName = new wxStaticText(this, wxID_ANY, wxT("育成ウマ娘"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextCharaName->Wrap(-1);
	fgSizerInfo->Add(m_staticTextCharaName, 0, wxALL, 5);

	m_comboBoxUma = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
	//m_comboBoxUma->SetMinSize(wxSize(300, -1));

	fgSizerInfo->Add(m_comboBoxUma, 1, wxEXPAND | wxALL, 5);

	m_staticTextEventName = new wxStaticText(this, wxID_ANY, wxT("イベント名"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticTextEventName->Wrap(-1);
	fgSizerInfo->Add(m_staticTextEventName, 0, wxALL, 5);

	m_textCtrlEventSource = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_READONLY);
	m_textCtrlEventSource->SetBackgroundColour(wxColour(255, 255, 255));

	fgSizerInfo->Add(m_textCtrlEventSource, 0, wxALL, 5);
	bSizer42->Add(fgSizerInfo, 0, wxEXPAND, 5);
	bSizerTop->Add(bSizer42, 0, wxEXPAND | wxRIGHT | wxLEFT, 5);

	wxStaticBoxSizer* sbSizerOptions = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("選択肢")), wxVERTICAL);

	// 選択肢1
	wxBoxSizer* bSizerOption1 = new wxBoxSizer(wxHORIZONTAL);
	m_textCtrlEvent1 = new wxTextCtrl(sbSizerOptions->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent1->SetBackgroundColour(wxColour(200, 255, 150));
	m_textCtrlEvent1->SetMinSize(wxSize(180, -1));
	bSizerOption1->Add(m_textCtrlEvent1, 0, wxALL, 5);

	m_richText1 = new wxUmaTextCtrl(sbSizerOptions->GetStaticBox());
	m_richText1->SetMinSize(wxSize(-1, 55));
	bSizerOption1->Add(m_richText1, 1, wxALL | wxEXPAND, 5);

	sbSizerOptions->Add(bSizerOption1, 1, wxEXPAND, 5);
	
	// 選択肢2
	wxBoxSizer* bSizerOption2 = new wxBoxSizer(wxHORIZONTAL);
	m_textCtrlEvent2 = new wxTextCtrl(sbSizerOptions->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent2->SetBackgroundColour(wxColour(255, 240, 140));
	m_textCtrlEvent2->SetMinSize(wxSize(180, -1));
	bSizerOption2->Add(m_textCtrlEvent2, 0, wxALL, 5);

	m_richText2 = new wxUmaTextCtrl(sbSizerOptions->GetStaticBox());
	m_richText2->SetMinSize(wxSize(-1, 55));
	bSizerOption2->Add(m_richText2, 1, wxEXPAND | wxALL, 5);

	sbSizerOptions->Add(bSizerOption2, 1, wxEXPAND, 5);

	// 選択肢3
	wxBoxSizer* bSizerOption3 = new wxBoxSizer(wxHORIZONTAL);
	m_textCtrlEvent3 = new wxTextCtrl(sbSizerOptions->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_textCtrlEvent3->SetBackgroundColour(wxColour(255, 200, 200));
	m_textCtrlEvent3->SetMinSize(wxSize(180, -1));
	bSizerOption3->Add(m_textCtrlEvent3, 0, wxALL, 5);

	m_richText3 = new wxUmaTextCtrl(sbSizerOptions->GetStaticBox());
	m_richText3->SetMinSize(wxSize(-1, 55));
	bSizerOption3->Add(m_richText3, 1, wxEXPAND | wxALL, 5);

	sbSizerOptions->Add(bSizerOption3, 1, wxEXPAND, 5);

	bSizerTop->Add(sbSizerOptions, 1, wxEXPAND | wxALL, 5);

	this->SetSizer(bSizerTop);
	this->Layout();
	this->Centre(wxBOTH);

	// イベントバインド
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
	m_buttonAbout->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickAbout, this);
}

MainFrame::~MainFrame()
{
	delete umaMgr;
}

void MainFrame::Init()
{
	umaMgr->Init();

	int r = 3;

	for (auto& rank : umaMgr->GetCharacters()) {
		m_comboBoxUma->Append(std::wstring(L"☆") + std::to_wstring(r));

		for (auto& chara : rank) {
			m_comboBoxUma->Append(chara->Name);
		}
		
		r--;
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
	if (!m_PreviewWindow) {
		m_PreviewWindow = new PreviewFrame(this);
		m_PreviewWindow->Bind(DROP_IMAGE, &MainFrame::OnPreviewDragFile, this);
	}

	wxPoint pos = this->GetPosition();
	wxSize size = this->GetSize();

	m_PreviewWindow->SetPosition(wxPoint(pos.x + size.x, pos.y));
	m_PreviewWindow->Show();
}

void MainFrame::OnClickSetting(wxCommandEvent& event)
{
	SettingDialog* frame = new SettingDialog(this, &umaMgr->config);
	if (frame->ShowModal() == 1) {

	}
	if (frame->IsUpdated()) {
		m_comboBoxUma->Clear();
		umaMgr->Reload();
		int r = 3;

		for (auto& rank : umaMgr->GetCharacters()) {
			m_comboBoxUma->Append(std::wstring(L"☆") + std::to_wstring(r));

			for (auto& chara : rank) {
				m_comboBoxUma->Append(chara->Name);
			}

			r--;
		}
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
		ChangeEventOptions(umaMgr->CurrentEvent);
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

void MainFrame::OnClickAbout(wxCommandEvent& event)
{
	AboutDialog* dialog = new AboutDialog(this);
	dialog->ShowModal();
	dialog->Destroy();
}

void MainFrame::OnPreviewDragFile(wxCommandEvent& event)
{
	if (m_PreviewWindow) {
		const auto& image = m_PreviewWindow->GetImage();

		Gdiplus::Bitmap* gimage = new Gdiplus::Bitmap(image.GetHBITMAP(), NULL);
		cv::Mat mat = Uma::BitmapToCvMat(gimage);

		EventSource* EventSrc = umaMgr->DetectEvent(mat);
		if (EventSrc) {
			ChangeEventOptions(EventSrc);
		}

		delete gimage;
	}
}

void MainFrame::ChangeEventOptions(EventSource* event)
{
	if (event) {
		m_textCtrlEventSource->SetValue(event->Name);
		wxTextCtrl* controls[3] = { m_textCtrlEvent1, m_textCtrlEvent2, m_textCtrlEvent3 };
		wxUmaTextCtrl* richCtrls[3] = { m_richText1, m_richText2, m_richText3 };

		for (int i = 0; i < 3; i++) {
			if (i < event->Options.size()) {
				controls[i]->SetValue(event->Options[i]->Title);
				richCtrls[i]->SetValue(event->Options[i]->Effect);
			}
			else {
				controls[i]->SetValue(wxT(""));
				richCtrls[i]->SetValue(wxT(""));
			}
		}
	}
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