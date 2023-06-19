#include "CombineImage.h"

#include <Windows.h>
#include <gdiplus.h>

#include <numeric>
#include <opencv2/core/types_c.h>
#include <wx/imagbmp.h>

#include "Config.h"
#include"utility.h"

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


CombineImage::CombineImage() : IsCapture(false), BarLength(0), CurrentScrollPos(0), msec(0)
{
}

CombineImage::~CombineImage()
{
	if (IsCapture) EndCapture();
}

void CombineImage::StartCapture()
{
	if (IsCapture) return;

	IsCapture = true;
	IsScanStarted = false;
	IsFirstScan = true;
	CurrentScrollPos = -1;
	DetectedY = -1;
	IsManualStop = false;

	TemplateImage = cv::Mat();
	PrevImage = cv::Mat();
	Images.clear();
	DetectedYLines.clear();
	BarLength = 0;

	while (IsCapture) {
		auto start = std::chrono::system_clock::now();
		Capture();
		auto end = std::chrono::system_clock::now();
		msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		if (16 - msec >= 0) Sleep(16 - msec);
	}

	IsCapture = false;
	TemplateImage = cv::Mat();
	PrevImage = cv::Mat();
	Images.clear();
	DetectedYLines.clear();
}

void CombineImage::EndCapture()
{
	if (!IsCapture) return;

	IsManualStop = true;
	//_EndCapture();
}

bool CombineImage::Combine()
{
	if (Images.size() == 0) return false;
	
	cv::Mat concat;

	status = Combining;

	for (int i = 0; i < Images.size(); i++) {
		auto& info = DetectedYLines[i];

		if (info.StartY == 0) {
			cv::vconcat(cv::Mat(Images[i], cv::Rect(0, 0, Images[i].size().width, info.NextDetectedY)), concat);
		}
		else {
			cv::vconcat(concat, cv::Mat(Images[i], cv::Rect(0, info.StartY, Images[i].size().width, info.NextDetectedY != -1 ? info.NextDetectedY - info.StartY : Images[i].size().height - info.StartY)), concat);
		}
	}

	if (!concat.empty()) {
		Config* config = Config::GetInstance();

		std::wstring directory = config->ScreenshotSavePath + L"\\";
		if (config->ScreenshotSavePath.empty()) {
			directory = utility::GetExeDirectory() + L"\\Screenshots\\";
		}

		std::wstring filename = L"combine_" + utility::GetDateTimeString();
		std::wstring savename = directory + filename + config->GetImageExtension();

		cv::cvtColor(concat, concat, cv::COLOR_BGR2RGB);
		IplImage img = cvIplImage(concat);
		wxImage img1 = wxImage(img.width, img.height, (unsigned char*)img.imageData, true);
		return img1.SaveFile(savename.c_str(), wxBITMAP_TYPE_PNG);
	}

	return false;
}

void CombineImage::_EndCapture()
{
	if (!IsCapture) return;

	IsCapture = false;
	status = Stop;
}

void CombineImage::Capture()
{
	if (!IsCapture) return;

	Gdiplus::Bitmap* image = UmaWindowCapture::ScreenShot();
	if (!image) {
		_EndCapture();
		return;
	}

	cv::Mat mat = BitmapToCvMat(image);
	cv::Mat bar;

	CutScrollbar(mat, bar);
	ScrollbarDetector scroll(bar);

	if (!IsScanStarted && scroll.GetBarLength() == 0) {
		_EndCapture();
		// 通常キャプチャ
		delete image;
		return;
	}
	else if (scroll.IsBegin() && !IsScanStarted) {
		IsScanStarted = true;
		status = Scanning;
	}
	else if (!scroll.IsBegin() && !IsScanStarted) {
		status = WaitForMovingScrollbarOnTop;
	}

	if (BarLength == 0) BarLength = scroll.GetBarLengthRatio();
	else if (std::abs(BarLength - scroll.GetBarLengthRatio()) >= 1) {
		if (IsManualStop) {
			_EndCapture();
		}
		delete image;
		return;
	}

	if (!PrevImage.empty() && (PrevImage.size().width != mat.size().width || PrevImage.size().height != mat.size().height)) {
		_EndCapture();
		delete image;
		return;
	}

	if (IsScanStarted) {
		bool IsLast = scroll.IsEnd() || IsManualStop;

		// テンプレートマッチを行う画像がある場合
		if (!TemplateImage.empty() && (IsLast || scroll.GetPos() > CurrentScrollPos)) {
			cv::Mat result;
			cv::Point MaxPt;
			double MaxVal;

			cv::Mat gray, gray2, grayTemp;
			cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
			cv::cvtColor(mat, gray2, cv::COLOR_BGR2GRAY);
			cv::cvtColor(TemplateImage, grayTemp, cv::COLOR_BGR2GRAY);

			cv::matchTemplate(gray, grayTemp, result, cv::TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &MaxVal, NULL, &MaxPt);

			MaxVal = std::round(MaxVal * 100.0) / 100.0;

			// 検出されなかったとき
			if (MaxVal < 0.95 || IsLast) {
				if (IsLast || DetectedY != -1) {
					Images.push_back(IsLast ? mat.clone() : PrevImage);
					DetectedYLines.emplace_back(IsLast ? MaxPt.y : DetectedY, -1);
					// スクロールバーが途中かつ手動停止でない場合は次のマッチさせる画像を取得
					if (!IsLast) {
						int y = GetTemplateImage(PrevImage, TemplateImage);
						if (y != -1) {
							DetectedYLines.back().NextDetectedY = y;
							DetectedY = -1;
						}
					}
					else {
						_EndCapture();
						Combine();
					}
				}
			}
			else {
				if (DetectedY == -1 || MaxPt.y < DetectedY) {
					DetectedY = MaxPt.y;
				}
			}

			PrevImage = mat.clone();
			CurrentScrollPos = scroll.GetPos();
		}
		else if (TemplateImage.empty()) {
			int y = GetTemplateImage(mat, TemplateImage);
			if (y != -1) {
				if (IsFirstScan) {
					Images.push_back(mat.clone());
					DetectedYLines.emplace_back(0, y);
					IsFirstScan = false;
					CurrentScrollPos = scroll.GetPos();
				}
			}
		}
	}

	if (IsManualStop) {
		_EndCapture();
	}

	delete image;
}

int CombineImage::GetTemplateImage(const cv::Mat& mat, cv::Mat& cut)
{
	cv::Mat bin;

	//CutRecognizeRange(mat, mat2);
	cv::inRange(mat, cv::Scalar(242, 243, 242), cv::Scalar(242, 243, 242), bin);

	/*
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
	*/
	cv::Rect rect(
		0.02452830188679245283018867924528 * mat.size().width, std::round(0.46235418875927889713679745493107 * mat.size().height),
		0.95283018867924528301886792 * mat.size().width, std::round(0.40402969247083775185577942735949 * mat.size().height)
	);

	int y = rect.y + rect.height;
	int start_y = y;
	int end_y;
	int count = 0;

	do {
		end_y = start_y;

		UINT8 last_color = bin.at<UINT8>(y, rect.x + rect.width / 4);
		if (last_color == 0) {
			while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 0) y--;
		}
		else {
			while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 255) y--;
			end_y = y;
			while (y >= 0 && bin.at<UINT8>(y, rect.x + rect.width / 4) == 0) y--;
		}

		if (y < 0) {
			cut = cv::Mat();
			return -1;
		}

		start_y = y;
		count++;
	} while (end_y - start_y <= 10 && count <= 1);

	

	// スクロールバーは除外したい
	cut = cv::Mat(mat, cv::Rect(rect.x, start_y, rect.width - ((rect.x + rect.width) - (int)(0.96544276457883369330453563714903 * mat.size().width) + 1), end_y - start_y)).clone();

	return start_y;
}

void CombineImage::CutRecognizeRange(const cv::Mat& mat, cv::Mat& out)
{
	out = cv::Mat(mat, cv::Rect(
		0.02452830188679245283018867924528 * mat.size().width, std::round(0.46235418875927889713679745493107 * mat.size().height),
		0.95283018867924528301886792 * mat.size().width, std::round(0.40827147401908801696712619300106 * mat.size().height)
	));
}

void CombineImage::CutScrollbar(const cv::Mat& src, cv::Mat& out)
{
	out = cv::Mat(src, cv::Rect(
		0.96544276457883369330453563714903 * src.size().width, std::round(0.47515151515151515151515151515152 * src.size().height),
		0.00862068965517241379310344827586 * src.size().width, std::round(0.38060606060606060606060606060606 * src.size().height)
	));
}
