#include "SettingDialog.h"

#include <WinInet.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>

#include "utility.h"

#include "Config.h"


SettingDialog::SettingDialog(wxWindow* parent, Config* config) : wxDialog(parent, wxID_ANY, wxT("�ݒ�"), wxDefaultPosition, wxSize(500, -1), wxDEFAULT_DIALOG_STYLE)
{
	this->bUpdated = false;
	this->config = config;

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));

	wxBoxSizer* sizeParent = new wxBoxSizer(wxVERTICAL);

	// ���
	wxStaticBoxSizer* sizeS1 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("���")), wxVERTICAL);
	wxBoxSizer* sizeSB = new wxBoxSizer(wxHORIZONTAL);

	m_staticTextUpdate = new wxStaticText(sizeS1->GetStaticBox(), wxID_ANY, wxT("�C�x���g�����ŐV�ɍX�V����"));
	m_buttonUpdate = new wxButton(sizeS1->GetStaticBox(), wxID_ANY, wxT("�X�V"));

	sizeSB->Add(m_staticTextUpdate, 0, wxALL, 5);
	sizeSB->Add(m_buttonUpdate, 0, wxALL, 5);

	sizeS1->Add(sizeSB, 1, wxALL, 5);
	sizeParent->Add(sizeS1, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 5);

	// �X�N���[���V���b�g
	wxStaticBoxSizer* sizeS2 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("�X�N���[���V���b�g")), wxVERTICAL);
	wxFlexGridSizer* fgSize = new wxFlexGridSizer(0, 3, 0, 0);
	fgSize->SetFlexibleDirection(wxBOTH);
	fgSize->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);

	m_staticTextScreenShotPath = new wxStaticText(sizeS2->GetStaticBox(), wxID_ANY, wxT("�ꏊ:"));
	m_textCtrlScreenShotPath = new wxTextCtrl(sizeS2->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(280, -1));
	m_buttonBrowse = new wxButton(sizeS2->GetStaticBox(), wxID_ANY, wxT("�Q��"));

	fgSize->Add(m_staticTextScreenShotPath, 0, wxALL, 5);
	fgSize->Add(m_textCtrlScreenShotPath, 0, wxALL, 5);
	fgSize->Add(m_buttonBrowse, 0, wxALL, 5);

	sizeS2->Add(fgSize, 1, wxEXPAND, 5);
	sizeParent->Add(sizeS2, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// ���x�ȋ@�\
	wxStaticBoxSizer* sizeS3 = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("���x�ȋ@�\")), wxVERTICAL);
	m_checkDebugEnable = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("�f�o�b�O��L���ɂ���"));
	m_checkDebugEnable->Enable(false);
	m_checkSaveScreenShot = new wxCheckBox(sizeS3->GetStaticBox(), wxID_ANY, wxT("�C�x���g�����ʂł��Ȃ������ꍇ�ɃL���v�`������"));
	m_checkSaveScreenShot->Enable(false);

	sizeS3->Add(m_checkDebugEnable, 0, wxALL, 5);
	sizeS3->Add(m_checkSaveScreenShot, 0, wxALL, 5);

	sizeParent->Add(sizeS3, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// �_�C�A���O�{�^��
	m_dialogButtonSizer = new wxStdDialogButtonSizer();
	wxButton* dbSizerOk = new wxButton(this, wxID_OK);
	m_dialogButtonSizer->SetAffirmativeButton(dbSizerOk);
	wxButton* dbSizerCancel = new wxButton(this, wxID_CANCEL, wxT("�L�����Z��"));
	m_dialogButtonSizer->SetCancelButton(dbSizerCancel);
	m_dialogButtonSizer->Realize();

	sizeParent->Add(m_dialogButtonSizer, 0, wxEXPAND | wxALL, 5);

	this->SetSizer(sizeParent);
	this->Layout();
	sizeParent->Fit(this);

	this->Centre(wxBOTH);

	this->Bind(wxEVT_INIT_DIALOG, &SettingDialog::OnInitDialog, this);
	m_buttonUpdate->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickUpdate, this);
	m_buttonBrowse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickBrowse, this);
	dbSizerOk->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SettingDialog::OnClickOkButton, this);
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::OnInitDialog(wxInitDialogEvent& event)
{
	m_textCtrlScreenShotPath->SetLabelText(config->ScreenshotSavePath.c_str());
}

void SettingDialog::OnClickUpdate(wxCommandEvent& event)
{
	if (UpdateLibrary()) {
		bUpdated = true;
		wxMessageBox(wxT("�X�V���������܂����B"), wxT("�E�}�E�}�`�F�b�J�["));
	}
}

void SettingDialog::OnClickBrowse(wxCommandEvent& event)
{
	wxDirDialog* dlg = new wxDirDialog(this, wxT("�X�N���[���V���b�g�ۑ����I��"), wxT(""), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dlg->ShowModal() == wxID_OK) {
		m_textCtrlScreenShotPath->SetValue(dlg->GetPath());
	}
	dlg->Destroy();
}

void SettingDialog::OnClickOkButton(wxCommandEvent& event)
{
	config->ScreenshotSavePath = m_textCtrlScreenShotPath->GetValue().wc_str();
	this->EndModal(1);
}

bool SettingDialog::UpdateLibrary()
{
	std::wstring directory = utility::GetExeDirectory() + L"\\Library\\";

	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Chara.json", directory + L"Chara.json");
	UpdateFile(L"https://raw.githubusercontent.com/Cilda/UmaUmaChecker/master/UmaUmaChecker/Library/Events.json", directory + L"Events.json");
	return true;
}

bool SettingDialog::UpdateFile(const std::wstring& url, const std::wstring& path)
{
	HINTERNET hInternetOpen = NULL;
	HINTERNET hInternetConnect = NULL;
	HINTERNET hInternetRequest = NULL;
	int Version;
	DWORD dwReadSize;

	URL_COMPONENTSW urlcomponents;
	wchar_t szHostName[256];
	wchar_t szUrlPath[256];
	DWORD bytes;

	// URL���
	ZeroMemory(&urlcomponents, sizeof(URL_COMPONENTSW));
	urlcomponents.dwStructSize = sizeof(URL_COMPONENTSW);
	urlcomponents.lpszHostName = szHostName;
	urlcomponents.lpszUrlPath = szUrlPath;
	urlcomponents.dwHostNameLength = 256;
	urlcomponents.dwUrlPathLength = 256;

	if (!InternetCrackUrlW(url.c_str(), 0, 0, &urlcomponents)) {
		return false;
	}

	DWORD dwFlags = 0;
	if (urlcomponents.nScheme == INTERNET_SCHEME_HTTP) {
		dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_AUTO_REDIRECT;
	}
	else if (urlcomponents.nScheme == INTERNET_SCHEME_HTTPS) {
		dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_AUTO_REDIRECT;
	}

	hInternetOpen = InternetOpenW(L"downloader", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternetOpen == NULL) {
		return false;
	}

	hInternetConnect = InternetConnectW(hInternetOpen, urlcomponents.lpszHostName, urlcomponents.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (hInternetConnect == NULL) {
		InternetCloseHandle(hInternetOpen);
		return false;
	}

	hInternetRequest = HttpOpenRequestW(hInternetConnect, L"GET", urlcomponents.lpszUrlPath, NULL, NULL, NULL, dwFlags, NULL);
	if (hInternetRequest == NULL) {
		InternetCloseHandle(hInternetConnect);
		InternetCloseHandle(hInternetOpen);
		return false;
	}

	if (!HttpSendRequest(hInternetRequest, NULL, 0, NULL, 0)) {
		InternetCloseHandle(hInternetRequest);
		InternetCloseHandle(hInternetConnect);
		InternetCloseHandle(hInternetOpen);
		return false;
	}

	DWORD dwStatusCode = 0;
	DWORD dwStatusCodeLength = sizeof(dwStatusCode);

	if (!HttpQueryInfo(hInternetRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwStatusCodeLength, NULL))
		return false;

	if (dwStatusCode != HTTP_STATUS_OK) {
		return false;
	}

	DWORD dwSize = 0;
	DWORD dwMaxSizeLength = sizeof(dwSize);
	HttpQueryInfo(hInternetRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwSize, &dwMaxSizeLength, NULL);

	char* buf = new char[dwSize];

	ZeroMemory(buf, dwSize);
	InternetReadFile(hInternetRequest, buf, dwSize, &dwReadSize);

	HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytes;

		WriteFile(hFile, buf, dwSize, &bytes, NULL);
		CloseHandle(hFile);
	}

	delete[] buf;

	InternetCloseHandle(hInternetRequest);
	InternetCloseHandle(hInternetConnect);
	InternetCloseHandle(hInternetOpen);

	return true;
}
