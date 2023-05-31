#pragma once

namespace cv {
	class Mat;
}

class ScrollbarDetector
{
public:
	ScrollbarDetector(cv::Mat& img);
	~ScrollbarDetector();

	int GetPos() const;
	bool IsBegin() const;
	bool IsEnd() const;

private:
	void InitScrollInfo(cv::Mat& img);

private:
	int TotalLength;
	int Length;
};

