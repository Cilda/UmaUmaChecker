#include "CombineImage.h"

#include <Windows.h>
#include <gdiplus.h>

#include <numeric>

#include "ScrollbarDetector.h"
#include "UmaWindowCapture.h"


cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image)
{
	assert(image->GetPixelFormat() == PixelFormat24bppRGB);
	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, image->GetPixelFormat(), &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC3, data.Scan0, data.Stride);

	image->UnlockBits(&data);
	return mat;
}


CombineImage::CombineImage() : IsCapture(false)
{
}

CombineImage::~CombineImage()
{
}

void CombineImage::StartCapture()
{
	if (IsCapture) return;

	IsCapture = true;
	IsScanStarted = false;
	IsFirstScan = true;
	CurrentScrollPos = -1;
	DetectedY = -1;

	TemplateImage = cv::Mat();
	PrevImage = cv::Mat();
	Images.clear();
	DetectedYLines.clear();
	BarLength = 0;

	thread = std::thread([&] {
		while (IsCapture) {
			auto start = std::chrono::system_clock::now();
			Capture();
			auto end = std::chrono::system_clock::now();
			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			if (16 - msec >= 0) Sleep(16 - msec);
		}
	});
}

void CombineImage::EndCapture()
{
	if (!thread.joinable()) return;

	_EndCapture();

	thread.join();
}

bool CombineImage::Combine()
{
	if (Images.size() == 0) return false;

	cv::Mat concat;

	for (int i = 0; i < Images.size(); i++) {
		auto& info = DetectedYLines[i];

		if (info.StartY == 0) {
			cv::vconcat(cv::Mat(Images[i], cv::Rect(0, 0, Images[i].size().width, info.NextDetectedY)), concat);
		}
		else {
			cv::vconcat(concat, cv::Mat(Images[i], cv::Rect(0, info.StartY, Images[i].size().width, info.NextDetectedY != -1 ? info.NextDetectedY - info.StartY : Images[i].size().height - info.StartY)), concat);
		}
	}

	return true;
}

void CombineImage::_EndCapture()
{
	if (!IsCapture) return;

	IsCapture = false;
}

void CombineImage::Capture()
{
	if (!IsCapture) return;

	Gdiplus::Bitmap* image = UmaWindowCapture::ScreenShot();
	if (!image) return;

	cv::Mat mat = BitmapToCvMat(image);
	cv::Mat bar;

	CutScrollbar(mat, bar);
	ScrollbarDetector scroll(bar);

	if (scroll.GetBarLength() == 0) {
		_EndCapture();
		// 通常キャプチャ
		delete image;
		return;
	}
	else if (scroll.IsBegin() && !IsScanStarted) {
		IsScanStarted = true;
	}

	if (BarLength == 0) BarLength = scroll.GetBarLengthRatio();
	else if (BarLength != scroll.GetBarLengthRatio()) {
		delete image;
		return;
	}

	if (IsScanStarted) {
		// テンプレートマッチを行う画像がある場合
		if (scroll.GetPos() > CurrentScrollPos && !TemplateImage.empty()) {
			cv::Mat result;
			cv::Point MaxPt;
			double MaxVal;

			cv::Mat gray, grayTemp;
			cv::cvtColor(mat, gray, cv::COLOR_RGB2GRAY);
			cv::cvtColor(TemplateImage, grayTemp, cv::COLOR_RGB2GRAY);

			cv::matchTemplate(gray, grayTemp, result, cv::TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &MaxVal, NULL, &MaxPt);

			// 検出されなかったとき
			if (MaxVal < 0.9 || scroll.IsEnd()) {
				if (DetectedY != -1) {
					Images.push_back(scroll.IsEnd() ? mat.clone() : PrevImage);
					DetectedYLines.emplace_back(scroll.IsEnd() ? MaxPt.y : DetectedY, -1);
					if (!scroll.IsEnd()) {
						int y = GetTemplateImage(PrevImage, TemplateImage);
						DetectedYLines.back().NextDetectedY = y;
						DetectedY = -1;
					}
				}
			}
			else {
				if (DetectedY == -1 || MaxPt.y < DetectedY) {
					DetectedY = MaxPt.y;
				}
			}

			PrevImage = mat.clone();
		}
		else if (BarLength == scroll.GetBarLengthRatio() && IsFirstScan) {
			int y = GetTemplateImage(mat, TemplateImage);
			Images.push_back(mat.clone());
			DetectedYLines.emplace_back(0, y);
			IsFirstScan = false;
		}

		CurrentScrollPos = scroll.GetPos();
		
		if (scroll.IsEnd()) {
			_EndCapture();
		}
	}

	delete image;
}

int CombineImage::GetTemplateImage(const cv::Mat& mat, cv::Mat& cut)
{
	cv::Mat bin;

	cv::inRange(mat, cv::Scalar(242, 243, 242), cv::Scalar(242, 243, 242), bin);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	double max_area = 0;
	std::vector<cv::Point> max;
	for (auto& contour : contours) {
		double area = cv::contourArea(contour);
		if (area > max_area) {
			max = contour;
		}
	}

	cv::Rect rect = cv::boundingRect(max);

	int y = rect.y + rect.height - 1;
	int start_y = -1;
	int end_y = -1;
	UINT8 last_color = bin.at<UINT8>(rect.y + rect.height - 1, rect.x + rect.width / 4);
	if (last_color == 0) {
		while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 0) y--;
	}
	else {
		while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 255) y--;
		while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 0) y--;
	}

	if (y < 0) return -1;

	start_y = y;
	end_y = rect.y + rect.height - 1;

	// スクロールバーは除外したい
	cut = cv::Mat(mat, cv::Rect(rect.x, start_y, (int)(0.96544276457883369330453563714903 * mat.size().width) - rect.x, end_y - start_y)).clone();

	return start_y;
}

void CombineImage::CutScrollbar(const cv::Mat& src, cv::Mat& out)
{
	out = cv::Mat(src, cv::Rect(
		0.96544276457883369330453563714903 * src.size().width, std::round(0.47515151515151515151515151515152 * src.size().height),
		0.00862068965517241379310344827586 * src.size().width, std::round(0.38060606060606060606060606060606 * src.size().height)
	));
}
