#include <winsock2.h>
#include "MainFrame.h"

#include <Windows.h>
#include <gdiplus.h>
#include <chrono>
#include <regex>
#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/log.h>

#include "utility.h"

#include "SettingDialog.h"
#include "AboutDialog.h"


MainFrame::MainFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size, long style) : wxFrame(parent, wxID_ANY, app_title, pos, size, style), umaMgr(new Uma(this)), m_PreviewWindow(NULL)
{
	this->SetIcon(wxICON(AppIcon));

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

	std::vector<wxColour> bgColors = {
		wxColour(200, 255, 150),
		wxColour(255, 240, 140),
		wxColour(255, 200, 200),
		wxColour(106, 227, 255),
		wxColour(182, 193, 255),
	};
	for (int i = 0; i < EventOptionCount; i++) {
		wxBoxSizer* bSizerOption1 = new wxBoxSizer(wxHORIZONTAL);
		wxTextCtrl* TitleCtrl = new wxTextCtrl(sbSizerOptions->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

		if (i < bgColors.size()) TitleCtrl->SetBackgroundColour(bgColors[i]);
		else TitleCtrl->SetBackgroundColour(*wxWHITE);

		TitleCtrl->SetMinSize(wxSize(180, -1));
		bSizerOption1->Add(TitleCtrl, 0, wxALL, 5);

		wxUmaTextCtrl* OptionCtrl = new wxUmaTextCtrl(sbSizerOptions->GetStaticBox());
		OptionCtrl->SetMinSize(wxSize(-1, 55));
		bSizerOption1->Add(OptionCtrl, 1, wxALL | wxEXPAND, 5);

		sbSizerOptions->Add(bSizerOption1, 1, wxEXPAND, 5);

		m_textCtrlEventTitles.push_back(TitleCtrl);
		m_textCtrlEventOptions.push_back(OptionCtrl);
	}

	bSizerTop->Add(sbSizerOptions, 1, wxEXPAND | wxALL, 5);

	this->SetSizer(bSizerTop);
	this->Fit();
	this->Layout();
	this->Centre(wxBOTH);

	// イベントバインド
	m_toggleBtnStart->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &MainFrame::OnClickStart, this);
	m_buttonScreenshot->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickScreenShot, this);
	m_buttonPreview->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickPreview, this);
	m_buttonSetting->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickSetting, this);
	m_comboBoxUma->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainFrame::OnSelectedUma, this);
	for (int i = 0; i < EventOptionCount; i++) {
		m_textCtrlEventOptions[i]->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnEnterControl, this);
		m_textCtrlEventOptions[i]->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnLeaveControl, this);
	}
	this->Bind(wxEVT_THREAD, &MainFrame::OnChangeUmaEvent, this);
	m_buttonAbout->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickAbout, this);

	Config* config = Config::GetInstance();
	if (config->WindowX != 0 || config->WindowY != 0) {
		this->Move(config->WindowX, config->WindowY);
	}

	Init();
}

MainFrame::~MainFrame()
{
	Config* config = Config::GetInstance();
	int x, y;

	GetPosition(&x, &y);

	config->WindowX = x;
	config->WindowY = y;

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

#ifdef _DEBUG
	new wxLogWindow(this, wxT("ログ"));
#endif
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
		Config* config = Config::GetInstance();

		std::wstring directory = config->ScreenshotSavePath + L"\\";
		if (config->ScreenshotSavePath.empty()) {
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
	if (m_PreviewWindow && m_PreviewWindow->IsShown()) return;

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
	Config* config = Config::GetInstance();
	SettingDialog* frame = new SettingDialog(this, config);
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

		for (int i = 0; i < EventOptionCount; i++) {
			if (i < event->Options.size()) {
				m_textCtrlEventTitles[i]->SetValue(event->Options[i]->Title);
				m_textCtrlEventOptions[i]->SetValue(event->Options[i]->Effect);
			}
			else {
				m_textCtrlEventTitles[i]->SetValue(wxT(""));
				m_textCtrlEventOptions[i]->SetValue(wxT(""));
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