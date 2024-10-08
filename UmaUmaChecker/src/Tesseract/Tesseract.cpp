#include "Tesseract.h"

#include "Utils/utility.h"
#include "Config/Config.h"

bool Tesseract::bInitialized = false;
object_pool<tesseract::TessBaseAPI> Tesseract::tess_pool;

void Tesseract::Initialize()
{
	if (bInitialized) return;

	auto instance = Config::GetInstance();
	int pool_size = std::max(instance->OcrPoolSize, 1);

	for (int i = 0; i < pool_size; i++) {
		tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
		api->Init(utility::to_string(utility::GetExeDirectory() + L"\\tessdata").c_str(), utility::to_string(instance->TesseractLanguage).c_str());
		api->SetPageSegMode(tesseract::PSM_RAW_LINE);

		tess_pool.join_manage_resource(api);
	}

	bInitialized = true;
}

void Tesseract::Uninitialize()
{
	tess_pool.clear();
}

std::wstring Tesseract::RecognizeAsRaw(const cv::Mat& image)
{
	if (!bInitialized) return L"";

	auto api = tess_pool.get();

	api->SetImage(image.data, image.size().width, image.size().height, image.channels(), image.step1());
	api->Recognize(NULL);

	const std::unique_ptr<const char[]> utf8_text(api->GetUTF8Text());
	std::wstring text = utility::from_u8string(utf8_text.get());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	tess_pool.release(api);
	return text;
}

std::wstring Tesseract::Recognize(const cv::Mat& image)
{
	if (!bInitialized) return L"";

	auto api = tess_pool.get();

	api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
	api->SetImage(image.data, image.size().width, image.size().height, image.channels(), image.step1());
	api->Recognize(NULL);

	const std::unique_ptr<const char[]> utf8_text(api->GetUTF8Text());
	std::wstring text = utility::from_u8string(utf8_text.get());
	text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());

	api->SetPageSegMode(tesseract::PSM_RAW_LINE);
	tess_pool.release(api);
	return text;
}
