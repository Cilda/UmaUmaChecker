#include "UmaTripRecognizer.h"


cv::Rect2d UmaTripRecognizer::range = {
	0.02777777777777777777777777777778,
	0.2890625,
	0.93849206349206349206349206349206,
	0.37276785714285714285714285714286
};

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

	/*
	auto m2 = img.clone();
	cv::drawContours(m2, contours, -1, cv::Scalar(0, 0, 255), 1);
	cv::imshow("test", m2);
	*/

	return L"";
}
