#pragma once

#include <wx/frame.h>
#include <wx/timer.h>

#include "CombineImage.h"

namespace cv {
	class Mat;
}
namespace Gdiplus {
	class Bitmap;
}

class DebugImageCombineFrame : public wxFrame
{
public:
	DebugImageCombineFrame(wxWindow* parent);
	~DebugImageCombineFrame();

private:
	cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);

private:
	void OnDropFiles(wxDropFilesEvent& event);
	void OnClickStartCapture(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()

private:
	wxTimer timer;
	CombineImage combine;
};

