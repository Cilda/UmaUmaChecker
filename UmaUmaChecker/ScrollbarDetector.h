#pragma once

namespace cv {
	class Mat;
}

class ScrollbarDetector
{
public:
	ScrollbarDetector(cv::Mat& img);
	~ScrollbarDetector();

	bool IsValid() const;
	int GetPos() const;
	int GetTotalLength() const;
	int GetBarLength() const;
	int GetBarLengthRatio() const;
	bool IsBegin() const;
	bool IsEnd() const;

private:
	void InitScrollInfo(cv::Mat& img);
	

private:
	bool valid;

	int TotalLength;
	int Length;

	int ScrollBarStartY;
	int ScrollBarEndY;

	int MinY;
	int MaxY;
};

