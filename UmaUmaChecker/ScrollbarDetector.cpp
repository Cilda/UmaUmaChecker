#include "ScrollbarDetector.h"

#include <mutex>

#include <opencv2/opencv.hpp>


Point<double> ScrollbarDetector::Start(0.967, 0.4519);
Point<double> ScrollbarDetector::End(0.967, 1.0 - 0.08);

ScrollbarDetector::ScrollbarDetector(const cv::Mat& img) : valid(false)
{
	DetectScrollbar(img);
	//InitScrollInfo(img);
}

ScrollbarDetector::~ScrollbarDetector()
{
}

bool ScrollbarDetector::IsValid() const
{
	return valid;
}

int ScrollbarDetector::GetPos() const
{
	if (!valid) return 0;

	return BarRange.end().y();
}

int ScrollbarDetector::GetTotalLength() const
{
	if (!valid) return 0;

	return ScrollRange.end().y() - ScrollRange.start().y();
}

int ScrollbarDetector::GetBarLength() const
{
	if (!valid) return 0;

	return BarRange.end().y() - BarRange.start().y();
}

bool ScrollbarDetector::IsBegin() const
{
	if (!valid) return false;

	return ScrollRange.start().y() == BarRange.start().y();
}

bool ScrollbarDetector::IsEnd() const
{
	if (!valid) return false;

	return ScrollRange.end().y() == BarRange.end().y();
}

void ScrollbarDetector::DetectScrollbar(const cv::Mat& img)
{
	valid = false;

	auto margin = GetMargin(img);
	if (margin[0].y() == 0 || margin[1].y() == 0)
		return;

	// スクロールバーの範囲が取れたのでスクロールバーの長さを取得
	auto bar = GetScrollBar(img, margin[0], margin[1]);
	if (bar[0].y() == 0 || bar[1].y() == 0)
		return;

	ScrollRange = Range<Point<int>>(margin[0], margin[1]);
	BarRange = Range<Point<int>>(bar[0], bar[1]);

	valid = true;
}

std::vector<Point<int>> ScrollbarDetector::GetMargin(const cv::Mat& img)
{
	auto start = Point<int>(img.size().width * Start.x(), img.size().height * Start.y());
	auto end = Point<int>(img.size().width * End.x(), img.size().height * End.y());

	std::once_flag once_start;
	Point<int> pos_start(start.x(), 0);
	Point<int> pos_end(start.x(), 0);
	bool scanning = false;

	for (int y = start.y(); y < end.y(); y++) {
		auto& c = img.at<cv::Vec3b>(y, start.x());
		if (c[0] >= 140 && c[1] >= 121 && c[2] >= 123 &&
			c[0] <= 217 && c[1] <= 210 && c[2] <= 211) {
			std::call_once(once_start, [&] {
				pos_start.y(y);
			});
			pos_end.y(y);
			scanning = true;
		}
		else if (scanning) break;
	}

	return {
		pos_start,
		pos_end
	};
}

std::vector<Point<int>> ScrollbarDetector::GetScrollBar(const cv::Mat& img, const Point<int>& start, const Point<int>& end)
{
	std::once_flag once;
	Point<int> pos_start(start.x(), 0);
	Point<int> pos_end(start.x(), 0);
	bool scanning = false;

	for (int y = start.y(); y <= end.y(); y++) {
		auto& c = img.at<cv::Vec3b>(y, start.x());

		if (c[0] >= 140 && c[1] >= 121 && c[2] >= 123 &&
			c[0] <= 210 && c[1] <= 210 && c[2] <= 210) {
			std::call_once(once, [&] {
				pos_start.y(y);
			});
			pos_end.y(y);
			scanning = true;
		}
		else if (scanning)
			break;
	}

	return {
		pos_start,
		pos_end
	};
}
