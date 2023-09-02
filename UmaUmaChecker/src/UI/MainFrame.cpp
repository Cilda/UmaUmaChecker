#include <winsock2.h>
#include <Windows.h>
#include <gdiplus.h>

#include "MainFrame.h"

#include <sstream>
#include <fstream>
#include <regex>
#include <nlohmann/json.hpp>

#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/log.h>

#include "System/SystemUsage.h"
#include "Utils/utility.h"

#include "SettingDialog.h"
#include "AboutDialog.h"
#include "GrandLiveMusicListFrame.h"
#include "DebugFrame.h"
#include "DebugImageCombineFrame.h"

#include "Theme/StdRenderer.h"
#include "Theme/DarkThemeRenderer.h"

#include "Log/Log.h"
#include "Capture/UmaWindowCapture.h"

#pragma comment(lib, "gdiplus.lib")

using json = nlohmann::json;


MainFrame::MainFrame(wxWindow* parent, const wxPoint& pos, const wxSize& size, long style) : ThemedWindowWrapper<wxFrame>(parent, wxID_ANY, app_title, pos, size, style), umaMgr(new Uma(this)), m_PreviewWindow(NULL)
{
	Config* config = Config::GetInstance();

	this->SetIcon(wxICON(AppIcon));
	this->SetFont(wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));
	this->SetDoubleBuffered(true);

	ChangeTheme();

	wxBoxSizer* bSizerTop = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* bSizerButtons = new wxBoxSizer(wxHORIZONTAL);

	m_toggleBtnStart = new ThemedButtonWrapper<wxToggleButton>(this, wxID_ANY, wxT("スタート"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_toggleBtnStart, 0, wxALL, 5);

	m_buttonScreenshot = new ThemedButtonWrapper<wxBitmapButton>(this, wxID_ANY, wxIcon(wxT("ScreenShot"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
	m_buttonScreenshot->SetToolTip(wxT("スクリーンショットを撮ります\n右クリックをすると保存先フォルダを開きます"));
	bSizerButtons->Add(m_buttonScreenshot, 0, wxALL & ~wxRIGHT, 5);

	m_buttonCombine = new ThemedButtonWrapper<wxBitmapButton>(this, wxID_ANY, wxIcon(wxT("StartRecord"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
	m_buttonCombine->SetToolTip(wxT("ウマ娘詳細を一つの画像として撮影を開始します"));
	bSizerButtons->Add(m_buttonCombine, 0, wxALL & ~wxLEFT, 5);

	m_buttonPreview = new ThemedButtonWrapper<wxButton>(this, wxID_ANY, wxT("プレビュー表示"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonPreview, 0, wxALL, 5);

	m_buttonSetting = new ThemedButtonWrapper<wxButton>(this, wxID_ANY, wxT("設定"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonSetting, 0, wxALL, 5);

	m_buttonAbout = new ThemedButtonWrapper<wxButton>(this, wxID_ANY, wxT("About"), wxDefaultPosition, wxDefaultSize, 0);
	bSizerButtons->Add(m_buttonAbout, 0, wxALL, 5);

	bSizerTop->Add(bSizerButtons, 0, wxEXPAND | wxFIXED_MINSIZE | wxRIGHT | wxLEFT | wxBOTTOM, 5);

	wxFlexGridSizer* sizerEventInfo = new wxFlexGridSizer(2, 2, 9, 9);

	// 育成ウマ娘
	m_staticTextCharaName = new ThemedWrapper<wxStaticText>(this, wxID_ANY, wxT("育成ウマ娘"));
	m_staticTextCharaName->Wrap(-1);
	sizerEventInfo->Add(m_staticTextCharaName, 0, wxALIGN_CENTER_VERTICAL);

	m_comboBoxUma = new ThemedComboBoxWrapper<wxComboBox>(this, wxID_ANY);
	sizerEventInfo->Add(m_comboBoxUma, 1, wxEXPAND);

	// イベント名
	m_staticTextEventName = new ThemedWrapper<wxStaticText>(this, wxID_ANY, wxT("イベント名"));
	m_staticTextEventName->Wrap(-1);
	sizerEventInfo->Add(m_staticTextEventName, 0, wxALIGN_CENTER_VERTICAL);

	m_textCtrlEventSource = new ThemedEditWrapper<wxTextCtrl>(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	sizerEventInfo->Add(m_textCtrlEventSource, 1, wxEXPAND);
	sizerEventInfo->AddGrowableCol(1, 1);

	bSizerTop->Add(sizerEventInfo, 0, wxEXPAND | wxRIGHT | wxLEFT, 15);

	// 選択肢
	wxStaticBoxSizer* sbSizerOptions = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, wxT("選択肢")), wxVERTICAL);

	std::vector<wxColour> bgColors = {
		wxColour(200, 255, 150),
		wxColour(255, 240, 140),
		wxColour(255, 200, 200),
		wxColour(106, 227, 255),
		wxColour(182, 193, 255),
	};
	for (int i = 0; i < EventOptionCount; i++) {
		wxBoxSizer* bSizerOption1 = new wxBoxSizer(wxHORIZONTAL);

		// 選択肢名
		wxTextCtrl* TitleCtrl = new wxTextCtrl(sbSizerOptions->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
		if (i < bgColors.size()) TitleCtrl->SetBackgroundColour(bgColors[i]);
		else TitleCtrl->SetBackgroundColour(*wxWHITE);
		bSizerOption1->Add(TitleCtrl, 2, wxALL, 5);

		// 効果
		wxUmaTextCtrl* OptionCtrl = new ThemedEditWrapper<wxUmaTextCtrl>(sbSizerOptions->GetStaticBox());
		OptionCtrl->SetHeightByLine(config->OptionMaxLine);
		bSizerOption1->Add(OptionCtrl, 3, wxALL | wxEXPAND, 5);

		sbSizerOptions->Add(bSizerOption1, 1, wxEXPAND, 5);

		m_textCtrlEventTitles.push_back(TitleCtrl);
		m_textCtrlEventOptions.push_back(OptionCtrl);
	}

	bSizerTop->Add(sbSizerOptions, 1, wxEXPAND | wxALL, 5);

	// ステータス
	wxStaticBoxSizer* sbStatus = new wxStaticBoxSizer(new ThemedWrapper<wxStaticBox>(this, wxID_ANY, wxT("ステータス")), wxHORIZONTAL);
	wxString StatusNames[] = { wxT("スピード"), wxT("スタミナ"), wxT("パワー"), wxT("根性"), wxT("賢さ"), wxT("合計")};
	wxGridSizer* sizerStatus = new wxGridSizer(1, 6, 5, 5);

	for (int i = 0; i < sizeof(StatusNames) / sizeof(wxString); i++) {
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

		sizer->Add(new ThemedWrapper<wxStaticText>(sbStatus->GetStaticBox(), wxID_ANY, StatusNames[i]));
		m_Statuses[i] = new ThemedEditWrapper<wxTextCtrl>(sbStatus->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(55, -1)), wxTE_READONLY);
		sizer->Add(m_Statuses[i], 1, wxEXPAND);

		sizerStatus->Add(sizer, 0, wxEXPAND);
	}

	sbStatus->Add(sizerStatus, 1, wxEXPAND);
	bSizerTop->Add(sbStatus, 0, wxEXPAND | (wxALL & ~wxTOP), 5);

	m_statusBar = new wxStatusBar(this, wxID_ANY);
	m_statusBar->SetFieldsCount(3);
	m_statusBar->PushStatusText(wxT("CPU: 0.0%"), 0);
	m_statusBar->PushStatusText(wxT("MEM: 0.0 MB"), 1);
	m_statusBar->PushStatusText(wxT("ウマ娘: 未検出"), 2);
	this->SetStatusBar(m_statusBar);

	m_comboPopup = new wxComboBoxPopup(this);

	this->SetSizer(bSizerTop);
	this->Fit();
	this->Layout();
	this->Centre(wxBOTH);

	this->SetSizeHints(wxSize(-1, this->GetSize().y), wxSize(-1, this->GetSize().y));
	this->SetSize(config->WindowWidth, this->GetSize().y);

	// イベントバインド
	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	m_toggleBtnStart->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &MainFrame::OnClickStart, this);
	m_buttonScreenshot->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickScreenShot, this);
	m_buttonScreenshot->Bind(wxEVT_RIGHT_DOWN, &MainFrame::OnRightClickScreenShot, this);
	m_buttonCombine->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickCombine, this);
	m_buttonPreview->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickPreview, this);
	m_buttonSetting->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickSetting, this);
	m_comboBoxUma->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &MainFrame::OnSelectedUma, this);
	for (int i = 0; i < EventOptionCount; i++) {
		m_textCtrlEventOptions[i]->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnEnterControl, this);
		m_textCtrlEventOptions[i]->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnLeaveControl, this);
	}
	this->Bind(wxEVT_THREAD, &MainFrame::OnUmaThreadEvent, this);
	m_buttonAbout->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &MainFrame::OnClickAbout, this);
	timer.Bind(wxEVT_TIMER, &MainFrame::OnTimer, this);
	this->Bind(wxEVT_DPI_CHANGED, &MainFrame::OnDPIChanged, this);

	// コンボボックスポップアップ用
	m_comboBoxUma->Bind(wxEVT_COMMAND_TEXT_UPDATED, &MainFrame::OnComboTextUpdate, this);
	m_comboPopup->Bind(wxEVT_HIDE, &MainFrame::OnSelectedListBoxItem, this);
	m_comboBoxUma->Bind(wxEVT_KEY_DOWN, &MainFrame::OnComboKeyDown, this);
	CombineTimer.Bind(wxEVT_TIMER, &MainFrame::OnCombineTimer, this);

	if (config->WindowX != 0 || config->WindowY != 0) {
		this->Move(config->WindowX, config->WindowY);
	}

	if (!config->IsShowStatusBar) m_statusBar->Hide();
	else timer.Start(1000);

	Init();
}

MainFrame::~MainFrame()
{
	Config* config = Config::GetInstance();
	int x, y;

	GetPosition(&x, &y);

	config->WindowX = x;
	config->WindowY = y;
	config->WindowWidth = GetSize().x;

	delete umaMgr;
}

void MainFrame::Init()
{
	umaMgr->Init();

	int r = 1;

	SetTrainingCharaComboBox();

	if (!LoadSkills()) wxMessageBox(wxT("Skills.json を読み込めませんでした。"), app_name, wxICON_ERROR);

	/*
	if (Config::GetInstance()->EnableDebug) {
		m_DebugFrame = new DebugFrame(this);
		m_DebugFrame->Show();
	}
	*/

#ifdef _DEBUG
	new wxLogWindow(this, wxT("ログ"));
	DebugFrame* debug = new DebugFrame(this);
	debug->Show();

	DebugImageCombineFrame* debug2 = new DebugImageCombineFrame(this);
	debug2->Show();
	
#endif
}

bool MainFrame::LoadSkills()
{
	std::fstream stream(utility::GetExeDirectory() + L"\\Library\\Skills.json");
	if (stream.good()) {
		std::stringstream text;

		text << stream.rdbuf();

		try {
			json skills = json::parse(text.str());

			for (auto skill : skills) {
				SkillMap[utility::from_u8string(skill["Name"].get<std::string>())] = utility::from_u8string(skill["Description"].get<std::string>());
			}
		}
		catch (json::exception& ex) {
			return false;
		}

		return true;
	}

	return false;
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	if (thread.joinable()) {
		wxMessageBox(wxT("終了するには結合を停止してください。"), wxT("エラー"), wxICON_ERROR);
		return;
	}

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
	Gdiplus::Bitmap* image = UmaWindowCapture::ScreenShot();
	if (image) {
		CLSID clsid;
		Config* config = Config::GetInstance();

		std::wstring directory = config->ScreenshotSavePath + L"\\";
		if (config->ScreenshotSavePath.empty()) {
			directory = utility::GetExeDirectory() + L"\\Screenshots\\";
		}

		std::wstring filename = L"screenshot_" + utility::GetDateTimeString();
		std::wstring savename = directory + filename + config->GetImageExtension();

		utility::GetEncoderClsid(config->GetImageMimeType().c_str(), &clsid);

		Gdiplus::EncoderParameters encoderParameters;
		ULONG quality = 100;

		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		if (image->Save(savename.c_str(), &clsid, &encoderParameters) != Gdiplus::Ok) {
			wxMessageBox(wxT("保存に失敗しました。"), wxT("ウマウマチェッカー"), wxICON_ERROR);
		}

		delete image;
	}
	else {
		wxMessageBox(wxT("ウマ娘のウィンドウが見つかりません。"), wxT("ウマウマチェッカー"), wxICON_ERROR);
	}
}

void MainFrame::OnRightClickScreenShot(wxMouseEvent& event)
{
	auto config = Config::GetInstance();
	wxString command = wxString::Format(wxT("explorer /root,%s"), config->ScreenshotSavePath.empty() ? utility::GetExeDirectory() + L"\\screenshots" : config->ScreenshotSavePath);
	wxExecute(command, wxEXEC_ASYNC, NULL);
}

void MainFrame::OnClickCombine(wxCommandEvent& event)
{
	if (!UmaWindowCapture::GetUmaWindow()) {
		wxMessageBox(wxT("ウマ娘のウィンドウが見つかりません。"), app_name, wxICON_ERROR);
		return;
	}

	if (combine.IsCapturing()) {
		combine.EndCapture();
	}
	else {
		CombineTimer.Start(100);

		bool bRunning = true;
		thread = std::thread([&] {
			combine.StartCapture();
			bRunning = false;
		});

		CombineStatus PrevStatus = Stop;

		while (bRunning) {
			if (PrevStatus != WaitForMovingScrollbarOnTop && combine.GetStatus() == WaitForMovingScrollbarOnTop) {
				m_buttonCombine->SetBitmap(wxIcon(wxT("WaitRecord"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
				m_buttonCombine->SetToolTip(wxT("ウマ娘詳細を一つの画像として撮影を停止します"));
			}
			else if (PrevStatus != Scanning && combine.GetStatus() == Scanning) {
				m_buttonCombine->SetBitmap(wxIcon(wxT("StopRecord"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
				m_buttonCombine->SetToolTip(wxT("ウマ娘詳細を一つの画像として撮影を停止します"));
			}

			PrevStatus = combine.GetStatus();
			wxYield();
		}

		if (thread.joinable()) thread.join();
		m_buttonCombine->SetBitmap(wxIcon(wxT("StartRecord"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
		m_buttonCombine->SetToolTip(wxT("ウマ娘詳細を一つの画像として撮影を開始します"));
		CombineTimer.Stop();
		this->SetTitle(app_title);
		if (!combine.IsImageSaved()) {
			std::wstring error = combine.GetError();
			if (!error.empty()) wxMessageBox(error, app_title, wxICON_ERROR);
			else wxMessageBox(wxT("キャプチャに失敗しました。"), app_title, wxICON_ERROR);
		}
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
	SettingDialog frame(this, config);
	if (frame.ShowModal() == 1) {
		if (!config->IsShowStatusBar) {
			timer.Stop();
			m_statusBar->Hide();
		}
		else if (!timer.IsRunning()) {
			m_statusBar->Show();
			PositionStatusBar();
			timer.Start(1000);
		}

		/*
		if (config->EnableDebug) {
			if (!m_DebugFrame) {
				m_DebugFrame = new DebugFrame(this);
				m_DebugFrame->Show();
			}
		}
		else {
			if (m_DebugFrame) {
				delete m_DebugFrame;
				m_DebugFrame = NULL;
			}
		}
		*/

		ChangeTheme();

		this->SetSizeHints(wxSize(this->GetSize().x, -1), wxDefaultSize);

		SetFontAllChildren(this, wxFont(config->FontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, config->FontName));
		for (auto ctrl : m_textCtrlEventOptions) {
			ctrl->SetHeightByLine(config->OptionMaxLine);
		}

		Fit();
		Layout();
		Refresh();

		this->SetSizeHints(wxSize(-1, this->GetSize().y), wxSize(-1, this->GetSize().y));
	}

	if (frame.IsUpdated()) {
		wxString Selected = m_comboBoxUma->GetStringSelection();
		m_comboBoxUma->Clear();
		umaMgr->Reload();
		SetTrainingCharaComboBox();

		if (!Selected.empty()) {
			if (umaMgr->SetTrainingCharacter(Selected.ToStdWstring()))
				m_comboBoxUma->SetStringSelection(Selected);
		}
	}
}

void MainFrame::OnSelectedUma(wxCommandEvent& event)
{
	if (!umaMgr->SetTrainingCharacter(m_comboBoxUma->GetValue().wc_str())) {
		//m_comboBoxUma->SetSelection(-1);
	}
}

void MainFrame::OnUmaThreadEvent(wxThreadEvent& event)
{
	if (event.GetId() == 1) {
		HBITMAP hBmp = event.GetPayload<HBITMAP>();

		if (umaMgr->CurrentEvent) {
			ChangeEventOptions(umaMgr->CurrentEvent);

			if (m_PreviewWindow) {
				BITMAP bmp;

				GetObject(hBmp, sizeof(BITMAP), &bmp);
				m_PreviewWindow->SetImage(hBmp, bmp.bmWidth, bmp.bmHeight);
			}
			else {
				DeleteObject(hBmp);
			}
		}
	}
	else if (event.GetId() == 2) {
		LOG_INFO << L"Recognized Training Character: " << event.GetString();

		m_comboBoxUma->SetStringSelection(event.GetString());
		umaMgr->SetTrainingCharacter(event.GetString().ToStdWstring());
	}
}

void MainFrame::OnEnterControl(wxMouseEvent& event)
{
	wxTextCtrl* ctrl = (wxTextCtrl*)event.GetEventObject();
	wxSize clientSize = ctrl->GetClientSize();
	wxSize windowSize = ctrl->GetSize();
	wxString text = ctrl->GetValue();

	if (text.empty()) return;

	wxTextAttr attr = ctrl->GetDefaultStyle();
	wxClientDC dc(ctrl);
	dc.SetFont(attr.GetFont());
	wxSize size = dc.GetMultiLineTextExtent(text);

	int marginWidth = windowSize.x - clientSize.x;
	int marginHeight = windowSize.y - clientSize.y;

	if (size.x > clientSize.x || size.y > clientSize.y) {
		int width = size.x > clientSize.x ? size.x + marginWidth : clientSize.x;
		int height = size.y > clientSize.y ? size.y : clientSize.y;

		wxPoint pos = ctrl->ClientToScreen(wxPoint(-2, -2));
		wxTextPopupCtrl* m_textPopup = new wxTextPopupCtrl(this, wxSize(width + marginWidth, height + marginHeight));
		m_textPopup->Position(pos, wxSize(0, 0));
		m_textPopup->SetSize(wxSize(width + marginWidth, height + marginHeight));
		m_textPopup->SetText(text);
		m_textPopup->Popup();
	}
}

void MainFrame::OnLeaveControl(wxMouseEvent& event)
{
}

void MainFrame::OnClickAbout(wxCommandEvent& event)
{
	AboutDialog dialog(this);
	dialog.ShowModal();
}

void MainFrame::OnPreviewDragFile(wxCommandEvent& event)
{
	if (m_PreviewWindow) {
		const auto& image = m_PreviewWindow->GetImage();

		Gdiplus::Bitmap* gimage = Gdiplus::Bitmap::FromHBITMAP(image.GetHBITMAP(), NULL);
		cv::Mat mat = Uma::BitmapToCvMat(gimage);

		EventSource* EventSrc = umaMgr->DetectEvent(mat);
		if (EventSrc) {
			ChangeEventOptions(EventSrc);
		}
#if _DEBUG
		else {
			EventRoot* root = umaMgr->DetectTrainingCharaName(mat);
			if (root) LOG_DEBUG << L"キャラ認識テスト[" << root->Name << L"]";
		}
#endif

		delete gimage;
	}
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
	auto& usage = SystemUsage::Get();

	double cpu_usage = usage.GetCpuUsage();
	size_t memory_usage = usage.MemoryUsage();

	m_statusBar->SetStatusText(wxString::Format(wxT("CPU: %.1lf%%"), cpu_usage));
	m_statusBar->SetStatusText(wxString::Format(wxT("MEM: %0.1f MB"), memory_usage / 1024.0 / 1024.0), 1);

	RECT rc;

	if (UmaWindowCapture::GetUmaWindow()) {
		::GetClientRect(UmaWindowCapture::GetUmaWindow(), &rc);
		m_statusBar->SetStatusText(wxString::Format(wxT("ウマ娘: %dx%d"), rc.right, rc.bottom), 2);
	}
	else {
		m_statusBar->SetStatusText(wxT("ウマ娘: 未検出"), 2);
	}
}

void MainFrame::OnCombineTimer(wxTimerEvent& event)
{
	CombineStatus status = combine.GetStatus();
	switch (status) {
		case Stop:
			break;
		case WaitForMovingScrollbarOnTop:
		case Scanning:
		case Combining:
			if (combine.GetProgressTime() > 0)
				this->SetTitle(wxString::Format(wxT("%s [%d fps]"), app_title, 1000 / combine.GetProgressTime()));
			break;
	}
}

void MainFrame::OnComboTextUpdate(wxCommandEvent& event)
{
	if (m_comboBoxUma->GetCurrentSelection() != wxNOT_FOUND) return;

	if (!event.GetString().empty()) {
		wxString value = event.GetString();

		if (!m_comboPopup->IsShown()) {
			wxPoint pos = m_comboBoxUma->ClientToScreen(wxPoint(0, m_comboBoxUma->GetSize().y));
			m_comboPopup->Position(pos, wxSize(0, 0));
			m_comboPopup->SetSize(wxSize(m_comboBoxUma->GetSize().x, this->FromDIP(100)));
			m_comboPopup->Popup(m_comboPopup);
		}

		m_comboPopup->ClearList();
		for (auto& rank : umaMgr->GetCharacters()) {
			for (auto& chara : rank) {
				if (chara->Name.find(value) != std::wstring::npos) {
					m_comboPopup->AddString(chara->Name);
				}
			}
		}
	}
	else if (m_comboPopup->IsShown()) {
		m_comboPopup->Hide();
	}
}

void MainFrame::OnSelectedListBoxItem(wxCommandEvent& event)
{
	wxString name = event.GetString();
	m_comboBoxUma->SetStringSelection(name);
	umaMgr->SetTrainingCharacter(m_comboBoxUma->GetValue().wc_str());
}

void MainFrame::OnComboKeyDown(wxKeyEvent& event)
{
	if (m_comboPopup->IsShown()) {
		if (event.m_keyCode == WXK_UP) {
			int index = m_comboPopup->m_listBox->GetSelection();
			if (index != wxNOT_FOUND) {
				if (index >= 1) {
					m_comboPopup->m_listBox->SetSelection(index - 1);
				}
			}
			else {
				if (m_comboPopup->m_listBox->GetCount() > 0) m_comboPopup->m_listBox->SetSelection(0);
			}
			return;
		}
		else if (event.m_keyCode == WXK_DOWN) {
			int index = m_comboPopup->m_listBox->GetSelection();
			if (index != wxNOT_FOUND) {
				if (index + 1 < m_comboPopup->m_listBox->GetCount()) {
					m_comboPopup->m_listBox->SetSelection(index + 1);
				}
			}
			else {
				if (m_comboPopup->m_listBox->GetCount() > 0) m_comboPopup->m_listBox->SetSelection(0);
			}
			return;
		}
		else if (event.m_keyCode == WXK_RETURN) {
			int index = m_comboPopup->m_listBox->GetSelection();
			if (index != wxNOT_FOUND) {
				wxString value = m_comboPopup->m_listBox->GetStringSelection();
				m_comboBoxUma->SetStringSelection(value.ToStdWstring());
				umaMgr->SetTrainingCharacter(value.ToStdWstring());
				m_comboPopup->Dismiss();
				return;
			}
		}
	}

	event.Skip();
}

void MainFrame::OnDPIChanged(wxDPIChangedEvent& event)
{
	LOG_INFO << "DPI was changed (NEW DPI -> " << event.GetNewDPI().x << ")";

	this->SetSizeHints(wxSize(this->GetSize().x, -1), wxDefaultSize);

	for (auto ctrl : m_textCtrlEventOptions) {
		ctrl->SetHeightByLine(Config::GetInstance()->OptionMaxLine);
		ctrl->Layout();
	}

	this->Fit();
	this->Layout();

	this->SetSizeHints(wxSize(-1, this->GetSize().y), wxSize(-1, this->GetSize().y));
}

void MainFrame::ChangeEventOptions(EventSource* event)
{
	if (event) {
		Config* config = Config::GetInstance();
		bool bShow = config->IsHideNoneChoise ? false : true;
		if (config->IsHideNoneChoise) {
			for (auto& option : event->Options) {
				if (!option->Title.empty() && option->Title != L"選択肢なし") {
					bShow = true;
					break;
				}
			}
		}

		if (!bShow) return;

		m_textCtrlEventSource->SetValue(event->Name);

		for (int i = 0; i < EventOptionCount; i++) {
			if (i < event->Options.size()) {
				std::wstring skill = GetSkillDescFromOption(event->Options[i]->Effect);

				m_textCtrlEventTitles[i]->SetValue(event->Options[i]->Title);
				if (skill.empty()) m_textCtrlEventOptions[i]->SetValue(event->Options[i]->Effect);
				else m_textCtrlEventOptions[i]->SetValue(event->Options[i]->Effect + L"\n==========================\n" + skill);

				
			}
			else {
				m_textCtrlEventTitles[i]->SetValue(wxT(""));
				m_textCtrlEventOptions[i]->SetValue(wxT(""));
			}
		}
	}
}

std::wstring MainFrame::GetSkillDescFromOption(const std::wstring& option)
{
	auto begin = option.begin();
	auto end = option.end();
	std::wregex regex(L"『(.+?)』");
	std::wsmatch match;
	std::wstring desc;
	std::set<std::wstring> SkillSet;

	while (std::regex_search(begin, end, match, regex)) {
		begin = match[0].second;
		auto name = match[1].str();

		if (SkillSet.find(name) == SkillSet.end() && SkillMap.find(name) != SkillMap.end()) {
			if (!desc.empty()) desc += L"\n\n";
			desc += L"《" + match[1].str() + L"》\n" + SkillMap.at(name);
			SkillSet.insert(name);
		}
	}

	return desc;
}

void MainFrame::ChangeTheme()
{
	auto config = Config::GetInstance();

	switch (config->Theme) {
		case 0:
		default:
			ThemeManager::SetTheme(wxCLASSINFO(StdRenderer));
			break;
		case 1:
			ThemeManager::SetTheme(wxCLASSINFO(DarkThemeRenderer));
			break;
	}
}

void MainFrame::SetTrainingCharaComboBox()
{
	int r = 3;

	auto& characters = umaMgr->GetCharacters();
	for (auto itr = characters.begin(); itr != characters.end(); itr++) {
		m_comboBoxUma->Append(std::wstring(L"☆") + std::to_wstring(r));

		for (auto& chara : *itr) {
			m_comboBoxUma->Append(chara->Name);
		}

		r--;
	}
}

void MainFrame::SetFontAllChildren(wxWindow* parent, const wxFont& font)
{
	if (!parent) return;

	parent->SetFont(font);

	wxWindowList& children = parent->GetChildren();

	for (wxWindowList::Node* node = children.GetFirst(); node; node = node->GetNext()) {
		wxWindow* current = node->GetData();
		SetFontAllChildren(current, font);
	}
}
