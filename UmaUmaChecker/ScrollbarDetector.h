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
	int GetTotalLength() const;
	int GetBarLength() const;
	int GetBarLengthRatio() const;
	bool IsBegin() const;
	bool IsEnd() const;

private:
	void InitScrollInfo(cv::Mat& img);

private:
	int TotalLength;
	int Length;
	int StartY;
	int EndY;
};

