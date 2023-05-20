#include "DebugFrame.h"



#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dynarray.h>
#include <wx/log.h>
#include <wx/dcclient.h>
#include <wx/rawbmp.h>

#include <Windows.h>
#include <gdiplus.h>

#include <opencv2/core/types_c.h>

#include "UmaTripRecognizer.h"

DebugFrame::DebugFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, wxT("デバッグウィンドウ"))
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxColour(255, 255, 255));
	this->DragAcceptFiles(true);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_comboType = new wxChoice(this, wxID_ANY);
	m_comboType->AppendString(wxT("cv::threshold"));
	m_comboType->AppendString(wxT("cv::adaptiveThreshold(ADAPTIVE_THRESH_MEAN_C)"));
	m_comboType->AppendString(wxT("cv::adaptiveThreshold(ADAPTIVE_THRESH_GAUSSIAN_C)"));
	m_comboType->SetSelection(0);
	sizer->Add(m_comboType, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_slider = new wxSlider(this, wxID_ANY, 2, 2, 255);
	sizer2->Add(new wxStaticText(this, wxID_ANY, wxT("blockSize:")), 0, wxALIGN_CENTER_VERTICAL);
	sizer2->Add(m_slider, 1);
	sizer->Add(sizer2, 0, wxEXPAND | wxALL, 5);

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_slider2 = new wxSlider(this, wxID_ANY, 2, 0, 255);
	m_checkBox = new wxCheckBox(this, wxID_ANY, wxT("マイナス"));
	sizer3->Add(new wxStaticText(this, wxID_ANY, wxT("C:")), 0, wxALIGN_CENTER_VERTICAL);
	sizer3->Add(m_slider2, 1);
	sizer3->Add(m_checkBox, 0);
	sizer->Add(sizer3, 0, wxEXPAND | wxALL, 5);

	m_range = new wxStaticText(this, wxID_ANY, wxT("blockSize: 3, C: 3"));
	sizer->Add(m_range, 0, wxEXPAND | wxALL, 5);

	//m_textCtrlOcrResult = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	//sizer->Add(m_textCtrlOcrResult, 1, wxEXPAND | wxALL, 5);

	m_panel = new wxPanel(this, wxID_ANY);
	m_panel->SetMinSize(wxSize(700, 120));
	sizer->Add(m_panel, 1, wxEXPAND);

	//m_buttonOcr = new wxButton(this, wxID_ANY, wxT("OCR"));
	//sizer->Add(m_buttonOcr, 0, wxALL, 5);

	this->SetSizer(sizer);
	this->Fit();
	this->Layout();
	this->Centre(wxBOTH);

	//m_buttonOcr->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DebugFrame::OnClickOcr, this);
	this->Bind(wxEVT_DROP_FILES, &DebugFrame::OnDropFile, this);
	m_comboType->Bind(wxEVT_CHOICE, &DebugFrame::OnComboChanged, this);
	m_checkBox->Bind(wxEVT_CHECKBOX, &DebugFrame::OnComboChanged, this);
	m_slider->Bind(wxEVT_SLIDER, &DebugFrame::OnSlider, this);
	m_slider2->Bind(wxEVT_SLIDER, &DebugFrame::OnSlider, this);
	m_panel->Bind(wxEVT_PAINT, &DebugFrame::OnPaint, this);
}

DebugFrame::~DebugFrame()
{
}

void DebugFrame::OnClickOcr(wxCommandEvent& event)
{
}

void DebugFrame::OnDropFile(wxDropFilesEvent& event)
{
	if (event.GetNumberOfFiles() > 0) {
		wxString filename = event.GetFiles()[0];

		if (wxFileExists(filename)) {
			cv::Mat img2 = cv::imread(filename.ToStdString());

			img = cv::Mat(img2, cv::Rect(
				0.15206185567010309278350515463918 * img2.size().width,
				0.18847603661820140010770059235326 * img2.size().height,
				0.61094941634241245136186770428016 * img2.size().width,
				0.02898550724637681159420289855072 * img2.size().height
			)).clone();

			double ratio = (int)(58.125 * 32) / (double)img2.size().height;
			cv::resize(img, img, cv::Size(), ratio, ratio, cv::INTER_CUBIC);

			cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
			cv::bitwise_not(img, img);

			FilterImage();

			m_panel->Refresh();
		}
	}
}

void DebugFrame::OnSlider(wxCommandEvent& event)
{
	FilterImage();
	m_panel->Refresh(false);
}

void DebugFrame::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(m_panel);

	if (!bin.empty()) {
		cv::Mat image;

		cv::cvtColor(bin, image, cv::COLOR_GRAY2RGB);
		IplImage img = cvIplImage(image);
		wxImage img1 = wxImage(image.size().width, image.size().height, (unsigned char*)img.imageData, true);
		wxBitmap bitmap(img1);

		dc.DrawBitmap(bitmap, 0, 0);
	}
}

void DebugFrame::OnComboChanged(wxCommandEvent& event)
{
	FilterImage();
	m_panel->Refresh();
}

void DebugFrame::FilterImage()
{
	int blockSize = m_slider->GetValue() / 2 * 2 + 1;
	int C = m_slider2->GetValue() * (m_checkBox->Get3StateValue() == wxCheckBoxState::wxCHK_CHECKED ? -1 : 1);

	m_range->SetLabelText(wxString::Format(wxT("blockSize: %d, C: %d"), blockSize, C));

	switch (m_comboType->GetSelection()) {
		case 0:
			cv::threshold(img, bin, 100, 255, cv::THRESH_OTSU);
			break;
		case 1:
			cv::adaptiveThreshold(img, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, blockSize <= 3 ? 5 : blockSize, C);
			break;
		case 2:
			cv::adaptiveThreshold(img, bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blockSize, C);
			break;
	}
}
