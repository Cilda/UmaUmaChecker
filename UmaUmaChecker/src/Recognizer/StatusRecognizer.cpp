#include "StatusRecognizer.h"

#include <future>

#include "tesseract/Tesseract.h"

void CutMat(const cv::Mat& src, const Rect<double>& rect, cv::Mat& out)
{
	out = cv::Mat(src, cv::Rect2d(src.size().width * rect.x, src.size().height * rect.y, src.size().width * rect.width, src.size().height * rect.height));
}

StatusRecognizer::StatusRecognizer(cv::Mat& img, StatusRecognizerConfig& config) : Config(config)
{
	image = img;
	Valid = false;
}

void StatusRecognizer::Recognize()
{
	cv::Mat speed, stamina, power, guts, wisdom;

	CutMat(image, Config.SpeedBounds, speed);
	CutMat(image, Config.StaminaBounds, stamina);
	CutMat(image, Config.PowerBounds, power);
	CutMat(image, Config.GutsBounds, guts);
	CutMat(image, Config.WisdomBounds, wisdom);



	{
		auto future1 = std::async(std::launch::async, [&] { Status.Speed = Tesseract::RecognizeAsNumber(speed); });
		auto future2 = std::async(std::launch::async, [&] { Status.Stamina = Tesseract::RecognizeAsNumber(stamina); });
		auto future3 = std::async(std::launch::async, [&] { Status.Power = Tesseract::RecognizeAsNumber(power); });
		auto future4 = std::async(std::launch::async, [&] { Status.Guts = Tesseract::RecognizeAsNumber(guts); });
		auto future5 = std::async(std::launch::async, [&] { Status.Wisdom = Tesseract::RecognizeAsNumber(wisdom); });
	}
}
