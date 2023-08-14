#include "UmaTripRecognizer.h"

#include <wx/log.h>

#include "Tesseract/Tesseract.h"


cv::Rect2d UmaTripRecognizer::range = {
	0.02777777777777777777777777777778,
	0.2890625,
	0.92849206349206349206349206349206,
	0.37276785714285714285714285714286
};
cv::Rect2d UmaTripRecognizer::TitleRange = {
	0.01388888888888888888888888888889,
	0.23102678571428571428571428571429,
	0.9781746031746031746031746031746,
	0.04799107142857142857142857142857
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

bool UmaTripRecognizer::Detect(cv::Mat& src, std::vector<std::wstring>& vec)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat bin;

	if (!IsDialog(src)) return L"";

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
			std::wstring name = Tesseract::RecognizeAsRaw(bin);
			vec.push_back(name);
		}
	}

	return vec.size() > 0;
}

bool UmaTripRecognizer::IsDialog(cv::Mat& src)
{
	cv::Mat img = cv::Mat(src, cv::Rect(
		TitleRange.x * src.size().width,
		TitleRange.y * src.size().height,
		TitleRange.width * src.size().width,
		TitleRange.height * src.size().height
	));
	cv::Mat bin;
	cv::inRange(img, cv::Scalar(5, 190, 103), cv::Scalar(15, 215, 150), bin);
	double ratio = cv::countNonZero(bin) / (double)bin.size().area();

	return ratio > 0.7;
}
