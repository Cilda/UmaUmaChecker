#pragma once

#include <opencv2/opencv.hpp>

namespace Gdiplus {
	class Bitmap;
}

class ImageConverter
{
public:
	ImageConverter() = delete;
	~ImageConverter() = delete;

	static cv::Mat BitmapToCvMat(Gdiplus::Bitmap* image);
};

