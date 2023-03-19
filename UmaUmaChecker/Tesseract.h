#pragma once

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "object_pool.hpp"

class Tesseract
{
public:
	Tesseract() = delete;
	~Tesseract() = delete;

	static void Initialize();
	static std::wstring Recognize(cv::Mat& image);

private:
	static bool bInitialized;
	static object_pool<tesseract::TessBaseAPI> tess_pool;
};

