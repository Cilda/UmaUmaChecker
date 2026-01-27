#include "SkillImageCombiner.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>

#include <wx/imagbmp.h>

#include "Processor/ScrollPositionProcessor.h"
#include "Images/ImageTemplateMatcher.h"
#include "Images/ImageConverter.h"
#include "Capture/UmaWindowCapture.h"


SkillImageCombiner::SkillImageCombiner() :
	minDetectVerticalPosition(-1), ScrollOnTop(false), processedScrollPos(-1), hasError(false)
{
}

SkillImageCombiner::~SkillImageCombiner()
{
}

bool SkillImageCombiner::Process(bool IsDestroy)
{
	std::unique_ptr<Gdiplus::Bitmap> image = std::unique_ptr<Gdiplus::Bitmap>(UmaWindowCapture::ScreenShot());
	if (!image) {
		SetError(L"Could not detect Umamusume window.");
		return false;
	}

	cv::Mat mat = ImageConverter::BitmapToCvMat(image.get());

	if (!frames.empty() && (mat.size().width != width || mat.size().height != height)) {
		SetError(L"Changed window size.");
		return false;
	}

	ScrollPositionProcessor scrollPos(mat);

	if (!scrollPos.IsValid()) {
		SetError(L"Could not find scrollbar.");
		return false;
	}
	else if (!ScrollOnTop) {
		if (scrollPos.IsBegin()) {
			ScrollOnTop = true;
		}
		else {
			return true;
		}
	}

	if (prevImg.empty()) {
		int y;
		templateImage = CutTemplateImage(mat, &y);

		if (y != -1) {
			frames.push_back(cv::Mat(mat.clone(), cv::Rect(0, 0, mat.size().width, y)));
			width = mat.size().width;
			height = mat.size().height;
			detectVerticalPositions.push_back(y);

			prevImg = mat.clone();
			processedScrollPos = scrollPos.GetPos();
			return true;
		}
	}

	if (scrollPos.GetPos() <= processedScrollPos && !IsDestroy) {
		return true;
	}

	if (scrollPos.IsEnd()) {
		ImageTemplateMatcher matcher(templateImage);
		double ratio = matcher.Match(mat);
		cv::Point point = matcher.GetMatchPoint();

		frames.push_back(cv::Mat(mat.clone(), cv::Rect(0, point.y, mat.size().width, mat.size().height - point.y)));
		detectVerticalPositions.push_back(-1);
		prevImg = cv::Mat();
		return false;
	}
	else {
		ImageTemplateMatcher matcher(templateImage);
		double ratio = matcher.Match(mat);
		cv::Point point = matcher.GetMatchPoint();
		const double minRecognizeY = std::round(0.46235418875927889713679745493107 * mat.size().height);

		if (ratio >= 0.90 && point.y >= minRecognizeY && !IsDestroy) {
			if (minDetectVerticalPosition == -1 || point.y < minDetectVerticalPosition)
				minDetectVerticalPosition = point.y;
		}
		else if (IsDestroy) {
			if (scrollPos.GetPos() >= processedScrollPos) {
				frames.push_back(cv::Mat(mat.clone(), cv::Rect(0, point.y, mat.size().width, mat.size().height - point.y)));
				detectVerticalPositions.push_back(point.y);
			}
			else {
				frames.push_back(cv::Mat(prevImg, cv::Rect(0, minDetectVerticalPosition, prevImg.size().width, prevImg.size().height - minDetectVerticalPosition)));
				detectVerticalPositions.push_back(minDetectVerticalPosition);
			}
		}
		else if (minDetectVerticalPosition != -1) {
			int y;

			templateImage = CutTemplateImage(prevImg, &y);

			if (y != -1) {
				frames.push_back(cv::Mat(prevImg, cv::Rect(0, minDetectVerticalPosition, mat.size().width, y - minDetectVerticalPosition)));
				detectVerticalPositions.push_back(minDetectVerticalPosition);
				minDetectVerticalPosition = -1;
			}
		}
	}

	processedScrollPos = scrollPos.GetPos();
	prevImg = mat.clone();

	return true;
}

bool SkillImageCombiner::Save(const std::filesystem::path& path) const
{
	cv::Mat combinedImage;

	for (auto& image : frames) {
		if (combinedImage.empty()) cv::vconcat(image, combinedImage);
		else cv::vconcat(combinedImage, image, combinedImage);
	}

	cv::cvtColor(combinedImage, combinedImage, cv::COLOR_BGR2RGB);
	IplImage img = cvIplImage(combinedImage);
	wxImage img1 = wxImage(img.width, img.height, (unsigned char*)img.imageData, true);
	return img1.SaveFile(path.wstring());
}

void SkillImageCombiner::Reset()
{
	detectVerticalPositions.clear();
	frames.clear();
	prevImg = cv::Mat();
	templateImage = cv::Mat();
	minDetectVerticalPosition = -1;
	ScrollOnTop = false;
	processedScrollPos = -1;
	hasError = false;
	message.clear();
}

cv::Mat SkillImageCombiner::CutTemplateImage(const cv::Mat& src, int* detectY)
{
	cv::Mat bin;
	cv::Rect rect(
		0.02452830188679245283018867924528 * src.size().width, std::round(0.46235418875927889713679745493107 * src.size().height),
		0.95283018867924528301886792 * src.size().width, std::round(0.40402969247083775185577942735949 * src.size().height)
	);
	int y = rect.y + rect.height;
	int start_y = y;
	int end_y;
	int count = 0;
	cv::Mat cut;

	cv::inRange(src, cv::Scalar(242, 243, 242), cv::Scalar(242, 243, 242), bin);

	do {
		end_y = start_y;

		unsigned char last_color = bin.at<unsigned char>(y, rect.x + rect.width / 4);
		if (last_color == 0) {
			while (y >= 0 && bin.at<unsigned char>(y, rect.x + rect.width / 4) == 0) y--;
		}
		else {
			while (y >= 0 && bin.at<unsigned char>(y, rect.x + rect.width / 4) == 255) y--;
			end_y = y;
			while (y >= 0 && bin.at<unsigned char>(y, rect.x + rect.width / 4) == 0) y--;
		}

		if (y < 0) {
			*detectY = -1;
			return {};
		}

		start_y = y;
		count++;
	} while (end_y - start_y <= 10 && count <= 1);

	// スクロールバーは除外したい
	cut = cv::Mat(src, cv::Rect(rect.x, start_y + 1, rect.width - ((rect.x + rect.width) - (int)(0.96544276457883369330453563714903 * src.size().width) + 1), end_y - start_y)).clone();
	*detectY = start_y;

	return cut;
}

void SkillImageCombiner::SetError(const std::wstring& message)
{
	hasError = true;
	this->message = message;
}
