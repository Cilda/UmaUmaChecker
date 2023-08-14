#pragma once

#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>

#include <opencv2/opencv.hpp>


class DebugFrame : public wxFrame
{
public:
	DebugFrame(wxWindow* parent);
	virtual ~DebugFrame();

private:
	void OnClickOcr(wxCommandEvent& event);
	void OnDropFile(wxDropFilesEvent& event);
	void OnSlider(wxCommandEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnComboChanged(wxCommandEvent& event);

	void FilterImage();

private:
	wxChoice* m_comboType;
	wxTextCtrl* m_textCtrlOcrResult;
	wxButton* m_buttonOcr;
	wxSlider* m_slider;
	wxSlider* m_slider2;
	wxPanel* m_panel;
	wxStaticText* m_range;
	wxCheckBox* m_checkBox;

	cv::Mat img;
	cv::Mat bin;
};

