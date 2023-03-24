#include "UmaTripRecognizer.h"

#include <wx/log.h>

#include "Tesseract.h"


cv::Rect2d UmaTripRecognizer::range = {
	0.02777777777777777777777777777778,
	0.2890625,
	0.93849206349206349206349206349206,
	0.37276785714285714285714285714286
};
double UmaTripRecognizer::StartXRatio = 0.183;
double UmaTripRecognizer::WidthRatio = 0.459;
double UmaTripRecognizer::HeightRatio = 0.41;

UmaTripRecognizer::UmaTripRecognizer()
{
}

UmaTripRecognizer::~UmaTripRecognizer()
{
}

std::wstring UmaTripRecognizer::Detect(cv::Mat& src)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat bin;

	auto img = cv::Mat(src, cv::Rect(range.x * src.size().width, range.y * src.size().height, range.width * src.size().width, range.height * src.size().height));
	
	cv::inRange(img, cv::Scalar(242, 243, 242), cv::Scalar(242, 243, 242), bin);
	cv::findContours(bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	// 先頭は画像自体の枠なので削除
	contours.erase(contours.begin());
	
	std::vector<cv::Rect> rects;
	for (auto& contour : contours) {
		auto rect = cv::boundingRect(contour);
		rects.push_back(rect);
	}

	for (auto& rect : rects) {
		int start_x = rect.x + rect.width * StartXRatio;
		int start_y = rect.y;
		int width = rect.width * WidthRatio;
		int height = rect.height * HeightRatio;

		auto img_name = cv::Mat(img, cv::Rect(start_x, start_y, width, height));
		cv::Mat gray, bin;

		cv::resize(img_name, img_name, cv::Size(), 2, 2, cv::INTER_CUBIC);
		cv::cvtColor(img_name, gray, cv::COLOR_RGB2GRAY);
		cv::threshold(gray, bin, 90, 255, cv::THRESH_BINARY);

		double black_ratio = (bin.size().area() - cv::countNonZero(bin)) / (double)bin.size().area();

		if (black_ratio > 0.01) {
			std::wstring name = Tesseract::Recognize(bin);
			wxLogDebug(wxT("%s"), name);
		}
	}

	return L"";
}
