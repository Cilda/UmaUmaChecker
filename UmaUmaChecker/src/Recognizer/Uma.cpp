//#define _WINSOCKAPI_
#include <winsock2.h>
#include <Windows.h>
//#include <WinInet.h>

#include "Uma.h"

#include <codecvt>
#include <crtdbg.h>
#include <future>
#include <numeric>

#include <wx/log.h>
#include <wx/utils.h>

#include "simstring/simstring.h"

#include "Log/Log.h"
#include "Utils/utility.h"
#include "Tesseract/Tesseract.h"
#include "Capture/UmaWindowCapture.h"
#include "EventDetector.h"


const cv::Rect2d Uma::EventTitleBound = { 0.15206185567010309278350515463918, 0.18847603661820140010770059235326, 0.61094941634241245136186770428016, 0.02898550724637681159420289855072 };
const cv::Rect2d Uma::BottomChoiseBound = { 0.1038, 0.6286, 0.8415, 0.04047 };
const cv::Rect2d Uma::TrainingCharaAliasNameBound = { 0.31715771230502599653379549393414, 0.11024390243902439024390243902439, 0.66897746967071057192374350086655, 0.02829268292682926829268292682927 };
const cv::Rect2d Uma::TrainingCharaNameBound = { 0.31715771230502599653379549393414, 0.1375609756097560975609756097561, 0.66897746967071057192374350086655, 0.02439024390243902439024390243902 };
const cv::Rect2d Uma::EventIconBound = { 0.15206185567010309278350515463918, 0.18551587301587301587301587301587, 0.06701940035273368606701940035273, 0.03769841269841269841269841269841 };
const cv::Rect2d Uma::StatusBounds[5] = {
	{ 0.1010638297872340425531914893617, 0.66916167664670658682634730538922, 0.08776595744680851063829787234043, 0.02095808383233532934131736526946 },
	{ 0.26063829787234042553191489361702, 0.66916167664670658682634730538922, 0.08776595744680851063829787234043, 0.02095808383233532934131736526946 },
	{ 0.41755319148936170212765957446809, 0.66916167664670658682634730538922, 0.08776595744680851063829787234043, 0.02095808383233532934131736526946 },
	{ 0.57180851063829787234042553191489, 0.66916167664670658682634730538922, 0.08776595744680851063829787234043, 0.02095808383233532934131736526946 },
	{ 0.73138297872340425531914893617021, 0.66916167664670658682634730538922, 0.08776595744680851063829787234043, 0.02095808383233532934131736526946 },
};
const cv::Rect2d Uma::OptionBounds[5] = {
	{ 0.1038, 0.2765, 0.8415, 0.04047 },
	{ 0.1038, 0.3633, 0.8415, 0.04047 },
	{ 0.1038, 0.4523, 0.8415, 0.04047 },
	{ 0.1038, 0.5412, 0.8415, 0.04047 },
	{ 0.1038, 0.6286, 0.8415, 0.04047 },
};

const double Uma::ResizeRatio = 2.0;
const float Uma::UnsharpRatio = 2.0f;

Uma::Uma(wxFrame* frame) : bDetected(false), bStop(false), thread(nullptr), CurrentCharacter(nullptr), CurrentEvent(nullptr)
{
	this->frame = frame;
}

Uma::~Uma()
{
	if (thread) {
		Stop();
	}
}

void Uma::Init()
{
	CurrentEvent = nullptr;
	CurrentCharacter = nullptr;

	Collector.Load();
}

bool Uma::Start()
{
	if (thread) return false;

	bStop = false;
	thread = new std::thread(&Uma::MonitorThread, this);

	LOG_INFO << "Started Recognizing";

	return true;
}

void Uma::Stop()
{
	if (!thread || !thread->joinable()) return;

	QueueUserAPC([](ULONG_PTR) {}, thread->native_handle(), 0);
	bStop = true;
	thread->join();
	delete thread;

	thread = nullptr;
	CurrentEvent = nullptr;

	LOG_INFO << "Stopped Recognizing";
}

bool Uma::SetTrainingCharacter(const std::wstring& CharaName)
{
	CurrentCharacter = EventLib.CharaEvent.GetName(CharaName).get();
	return CurrentCharacter != nullptr;
}

cv::Mat Uma::BitmapToCvMat(Gdiplus::Bitmap* image)
{
	assert(image->GetPixelFormat() == PixelFormat24bppRGB);
	Gdiplus::Rect rect(0, 0, image->GetWidth(), image->GetHeight());
	Gdiplus::BitmapData data;
	image->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, image->GetPixelFormat(), &data);

	cv::Mat mat = cv::Mat(image->GetHeight(), image->GetWidth(), CV_8UC3, data.Scan0, data.Stride);

	image->UnlockBits(&data);

	return mat;
}

cv::Mat Uma::ImageBinarization(cv::Mat& srcImg)
{
	cv::Mat gray;
	cv::Mat bin;

	cv::cvtColor(srcImg, gray, cv::COLOR_RGB2GRAY);
	cv::bitwise_not(gray, gray);
	//cv::threshold(gray, bin, 100, 255, cv::THRESH_OTSU);
	cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 77, 18);
	//cv::threshold(gray, bin, 236, 255, cv::THRESH_BINARY_INV);

	return bin.clone();
}

void Uma::MonitorThread()
{
	while (!bStop) {
		auto start = std::chrono::system_clock::now();

		Gdiplus::Bitmap* image = UmaWindowCapture::ScreenShot();
		if (image) {
			cv::Mat srcImage = BitmapToCvMat(image);
			bool bScaned = false;

			auto start1 = std::chrono::system_clock::now();

			uint64 hash = 0;
			std::vector<std::wstring> events;

			std::shared_ptr<EventSource> event = DetectEvent(srcImage, &hash, &events, &bScaned);
			if (event) {
				if (event.get() != CurrentEvent) {
					CurrentEvent = event.get();

					auto AdjustEvent = AdjustRandomEvent(event, srcImage);
					if (AdjustEvent) {
						event = AdjustEvent;
					}

					HBITMAP hBmp;

					image->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBmp);

					UmaThreadData data = { event, hBmp };

					wxThreadEvent event(wxEVT_THREAD);
					event.SetId(1);
					event.SetPayload(data);
					wxQueueEvent(frame, event.Clone());
				}
			}
			else if (!events.empty()) {
				OutputMissingEventInfo(image, hash, events);
			}

			auto end1 = std::chrono::system_clock::now();
			auto msec1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();

			wxLogDebug(wxT("DetectEvent(): %lld msec"), msec1);

			if (!bScaned) {
				auto start2 = std::chrono::system_clock::now();

				EventRoot* root = DetectTrainingCharaName(srcImage);
				if (root) {
					wxThreadEvent event(wxEVT_THREAD);
					event.SetId(2);
					event.SetString(root->Name);
					wxQueueEvent(frame, event.Clone());
				}

				auto end2 = std::chrono::system_clock::now();
				auto msec2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();

				wxLogDebug(wxT("DetectTrainingCharaName(): %lld msec"), msec2);
			}

			delete image;
		}

		auto end = std::chrono::system_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		//LOG_DEBUG << wxString::Format(wxT("MonitorThread() ループ処理時間: %lld msec"), msec);
		//OutputDebugStringW(wxString::Format(wxT("MonitorThread() ループ処理時間: %lld msec"), msec).wx_str());

		if (msec < 1000) SleepEx(1000 - msec, TRUE);
	}
}

std::vector<std::wstring> Uma::RecognizeCharaEventText(const cv::Mat& srcImg, uint64* pHash, bool HasEventIcon)
{
	cv::Mat gray, bin, blur;
	cv::Mat bin2;
	cv::Mat rsImg, rsImg2;
	cv::Mat cut = srcImg;

	if (HasEventIcon) {
		double x = Uma::EventIconBound.width * ImageSize.width;
		cut = cv::Mat(srcImg, cv::Rect(x, 0, cut.size().width - x, cut.size().height));
	}

	ResizeBest(cut, rsImg, ImageSize.height);
	//cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
	rsImg2 = rsImg.clone();
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::bitwise_not(gray, gray);

	bin = Uma::ImageBinarization(rsImg);
	RemoveWhiteSpace(bin, bin);
	cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

	bin2 = Uma::ImageBinarization(rsImg2);
	RemoveWhiteSpace(bin2, bin2);

	gray = cv::Mat(gray, cv::Rect(0, 0, bin.size().width, gray.size().height));
		
	std::vector<std::wstring> text_list;
	{
		auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
		auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
		auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });

		auto a4 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin2); });
	}

	AppendCollectedText(text_list);
	if (pHash) *pHash = GetImageHash(cut);

	return text_list;
}

bool Uma::IsBottomOption(const cv::Mat& srcImg)
{
	cv::Mat bg;
	cv::Mat img = srcImg.clone();

	cv::inRange(img, cv::Scalar(5, 40, 100), cv::Scalar(25, 70, 130), bg);
	return (double)cv::countNonZero(bg) / bg.size().area() > 0.01;
}

std::wstring Uma::GetTextFromImage(const cv::Mat& img)
{
	return Tesseract::RecognizeAsRaw(img);
}

int Uma::GetNumericFromImage(const cv::Mat& img)
{
	std::wstring value = Tesseract::RecognizeAsRaw(img);
	if (value.empty()) return 0;

	return std::stoi(value);
}

void Uma::AsyncFunction(std::vector<std::wstring>& strs, const cv::Mat& img)
{
	std::wstring str = GetTextFromImage(img);
	if (str.empty()) return;

	std::lock_guard<std::mutex> lock(mutex);
	strs.push_back(str);
}

void Uma::AppendCollectedText(std::vector<std::wstring>& text_list)
{
	std::vector<std::wstring> vecs;

	for (auto text : text_list) {
		if (Collector.Collect(text)) {
			vecs.push_back(text);
		}
	}

	text_list.insert(text_list.begin(), vecs.begin(), vecs.end());
}

void Uma::ChangeCollectedText(std::wstring& text)
{
	Collector.Collect(text);
}

double Uma::CalcTextMatchRate(const std::wstring& stext, const std::wstring& dtext)
{
	simstring::ngram_generator gen(1, false);

	double max_rate = 0.0;
	std::vector<std::wstring> basengrams;

	gen(stext, std::inserter(basengrams, basengrams.end()));


	std::vector<std::wstring> ngrams;
	int total = 0, equal = 0;

	gen(dtext, std::inserter(ngrams, ngrams.end()));

	for (auto& word1 : basengrams) {
		for (auto& word2 : ngrams) {
			if (word1 == word2) {
				equal++;
			}
		}

		total++;
	}

	return (double)equal / total;
}

void Uma::RemoveWhiteSpace(const cv::Mat& mat, cv::Mat& output)
{
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Point ret;

	cv::findContours(mat, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < contours.size() - 1; i++) {
		for (auto& pt : contours[i]) {
			if (pt.x > ret.x) ret.x = pt.x;
			if (pt.y > ret.y) ret.y = pt.y;
		}
	}

	if (ret.x + 10 > mat.size().width) output = mat;
	else output = cv::Mat(mat, cv::Rect(0, 0, ret.x + 10, mat.size().height));
}

void Uma::UnsharpMask(const cv::Mat& mat, cv::Mat& dst, float k)
{
	float kernelData[] = {
		-k / 9.0f, -k / 9.0f, -k / 9.0f,
		-k / 9.0f, 1 + (8 * k) / 9.0f, -k / 9.0f,
		-k / 9.0f, -k / 9.0f, -k / 9.0f
	};

	cv::Mat kernel(3, 3, CV_32F, kernelData);
	cv::filter2D(mat, dst, -1, kernel);
}

void Uma::ResizeBest(const cv::Mat& src, cv::Mat& dest, int height)
{
	// フォントサイズが30px～33pxだと一番識別率が良い
	double ratio = (int)(58.125 * 32) / (double)height;
	cv::resize(src, dest, cv::Size(), ratio, ratio, cv::INTER_CUBIC);
}

bool Uma::IsEventIcon(const cv::Mat& img)
{
	cv::Mat icon = cv::Mat(img, cv::Rect(
		Uma::EventIconBound.x * img.size().width,
		Uma::EventIconBound.y * img.size().height,
		Uma::EventIconBound.width * img.size().width,
		Uma::EventIconBound.height * img.size().height
	));
	cv::Mat gray, bin;

	cv::cvtColor(icon, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 140, 255, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Point ret;

	cv::findContours(bin, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (contours.size() > 0) {
		for (auto& contour : contours) {
			std::vector<cv::Point> approx;
			double arc_len = cv::arcLength(contour, true);
			cv::approxPolyDP(contour, approx, arc_len * 0.02, true);

			if (cv::isContourConvex(approx)) {
				cv::Rect rect = cv::boundingRect(approx);
				return std::abs(rect.width - rect.height) <= 2 && (double)rect.width / bin.size().width >= 0.75;
			}
		}
	}

	return false;
}

uint64 Uma::GetImageHash(const cv::Mat& img)
{
	cv::Mat resize;

	cv::resize(img, resize, cv::Size(9, 8));
	cv::cvtColor(resize, resize, cv::COLOR_RGB2GRAY);

	assert(resize.channels() == 1);

	uint64 hash = 0;
	unsigned int bit = 63;

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			int p1 = resize.at<unsigned char>(y, x);
			int p2 = resize.at<unsigned char>(y, x + 1);

			hash |= (uint64)(p1 < p2 ? 1 : 0) << bit;
			bit--;
		}
	}

	return hash;
}

int Uma::GetHashLength(uint64 hash1, uint64 hash2)
{
	uint64 d = hash1 ^ hash2;
	int length = 0;

	while (d) {
		d &= d - 1;
		length++;
	}

	return length;
}

void Uma::OutputMissingEventInfo(Gdiplus::Bitmap* image, uint64 hash, const std::vector<std::wstring>& events)
{
	if (PrevEventHash != hash) {
		Config* config = Config::GetInstance();

		PrevEventHash = hash;

		if (config->SaveMissingEvent) {
			std::wstring directory = config->ScreenshotSavePath + L"\\AutoSave\\";
			if (config->ScreenshotSavePath.empty()) {
				directory = utility::GetExeDirectory() + L"\\screenshots\\AutoSave\\";
			}

			CreateDirectoryW(directory.c_str(), NULL);

			std::wstring savename = directory
				+ std::wstring(L"debug_")
				+ std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
				+ L".png";

			CLSID clsid;
			utility::GetEncoderClsid(L"image/png", &clsid);
			image->Save(savename.c_str(), &clsid);
		}

		if (config->EnableDebug) {
			std::wstring str = std::accumulate(events.begin() + 1, events.end(), events[0], [](const std::wstring& a, const std::wstring& b) {
				if (b.empty()) return a;

				return a + L", " + b;
			});
			LOG_DEBUG << L"イベント不一致 識別文字[" << str << L"]";
			wxLogDebug(wxT("イベント不一致 識別文字[%s]"), str);
		}
	}
}

bool Uma::DetectCharaStatus(const cv::Mat& src)
{
	for (int i = 0; i < 5; i++) {
		cv::Mat cut = cv::Mat(src, cv::Rect(
			Uma::StatusBounds[i].x * src.size().width,
			Uma::StatusBounds[i].y * src.size().height,
			Uma::StatusBounds[i].width * src.size().width,
			Uma::StatusBounds[i].height * src.size().height
		));

		cv::Mat rsImg, gray, bin;

		cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::threshold(gray, bin, 120, 255, cv::THRESH_BINARY);

		double ratio = (double)cv::countNonZero(bin) / bin.size().area();

		//int status = GetNumericFromImage(bin);
		//int status2 = GetNumericFromImage(gray);
	}

	return false;
}

std::shared_ptr<EventSource> Uma::GetEventByBottomOption(const cv::Mat& srcImg)
{
	return RecognizeBottomOption(srcImg, &EventLib.CardEvent);
}

std::shared_ptr<EventSource> Uma::GetCharaEventByBottomOption(const cv::Mat& srcImg)
{

	return RecognizeBottomOption(srcImg, &EventLib.CharaEvent, CurrentCharacter);
}

std::shared_ptr<EventSource> Uma::GetScenarioEventByBottomOption(const cv::Mat& srcImg)
{

	return RecognizeBottomOption(srcImg, &EventLib.ScenarioEvent);
}

std::vector<std::wstring> Uma::RecognizeAllEventTitles(EventSource* event, const cv::Mat& img, BaseData* data)
{
	std::vector<cv::Mat> optionImages;

	for (int i = 0; i < 5; i++) {
		cv::Mat rsImg, gray, bin;
		cv::Mat cut = cv::Mat(img, cv::Rect(
			Uma::OptionBounds[i].x * img.size().width,
			Uma::OptionBounds[i].y * img.size().height,
			Uma::OptionBounds[i].width * img.size().width,
			Uma::OptionBounds[i].height * img.size().height
		));

		if (!IsBottomOption(cut)) {
			break;
		}

		ResizeBest(cut, rsImg, img.size().height);
		cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
		cv::threshold(gray, bin, 90, 255, cv::THRESH_BINARY);

		optionImages.push_back(bin);
	}

	std::vector<std::wstring> results;

	results.resize(optionImages.size());

	for (int i = 0; i < optionImages.size(); i++) {
		std::async(std::launch::async, [&] {
			auto text = Tesseract::RecognizeAsRaw(optionImages[i]);
			if (!text.empty()) {
				auto fixedText = data->RetrieveOptionTitle(text);

				if (!fixedText.empty()) {
					results[i] = fixedText;
				}
			}
		});
	}

	return results;
}

std::shared_ptr<EventSource> Uma::DetectEvent(const cv::Mat& srcImg, uint64* pHash, std::vector<std::wstring>* pEvents, bool* bScaned)
{
	if (bScaned) *bScaned = false;
	if (pHash) *pHash = 0;

	uint64 hash = 0;
	Config* config = Config::GetInstance();
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::EventTitleBound.x * srcImg.size().width,
		Uma::EventTitleBound.y * srcImg.size().height,
		Uma::EventTitleBound.width * srcImg.size().width,
		Uma::EventTitleBound.height * srcImg.size().height
	));
	EventDetector detector(cut);
	auto EventType = detector.GetEventType();

	ImageSize = srcImg.size();

	if (EventType == EventDetector::SupportCard) {
		// サポートカードイベント
		std::vector<std::wstring> events = RecognizeCardEventText(cut, &hash);
		if (!events.empty()) {
			if (bScaned) *bScaned = true;
			if (pEvents) *pEvents = events;
			if (pHash) *pHash = hash;

			auto event = GetCardEvent(events);
			if (!event || EventLib.CardEvent.IsEventNameDuplicate(event->Name)) event = GetEventByBottomOption(srcImg);
			if (config->EnableDebug && event && event.get() != CurrentEvent) LOG_DEBUG << L"[サポートカード] イベント名: " << event->Name;
			return event;
		}
	}
	else if (EventType == EventDetector::Character) {
		if (CurrentCharacter) {
			std::vector<std::wstring> events = RecognizeCharaEventText(cut, &hash, IsEventIcon(srcImg));
			if (!events.empty()) {
				if (bScaned) *bScaned = true;
				if (pEvents) *pEvents = events;
				if (pHash) *pHash = hash;

				auto event = GetCharaEvent(events);
				if (!event) event = GetCharaEventByBottomOption(srcImg);
				if (config->EnableDebug && event && event.get() != CurrentEvent) LOG_DEBUG << L"[育成ウマ娘] イベント名: " << event->Name;
				return event;
			}
		}
	}
	else if (EventType == EventDetector::Scenario) {
		std::vector<std::wstring> events = RecognizeScenarioEventText(cut, &hash);
		if (!events.empty()) {
			if (bScaned) *bScaned = true;
			if (pEvents) *pEvents = events;
			if (pHash) *pHash = hash;

			auto event = GetScenarioEvent(events);
			if (!event) event = GetScenarioEventByBottomOption(srcImg);
			if (config->EnableDebug && event && event.get() != CurrentEvent) LOG_DEBUG << L"[シナリオイベント] イベント名: " << event->Name;

			return event;
		}
	}

	return nullptr;
}

std::shared_ptr<EventSource> Uma::AdjustRandomEvent(std::shared_ptr<EventSource> event, const cv::Mat& image)
{
	if (EventLib.RandomEvent.IsScenarioRandom(event->Name)) {
		auto titles = RecognizeAllEventTitles(event.get(), image, &EventLib.ScenarioEvent);

		std::shared_ptr<EventSource> newEvent(new EventSource());
		newEvent->Name = event->Name;

		for (auto& title : titles) {
			for (auto& option : event->Options) {
				if (option->Title == title) {
					std::shared_ptr<EventOption> newOption(new EventOption());
					newOption->Title = option->Title;
					newOption->Effect = option->Effect;

					newEvent->Options.push_back(newOption);
					break;
				}
			}
		}

		return newEvent;
	}

	return nullptr;
}

EventRoot* Uma::DetectTrainingCharaName(const cv::Mat& srcImg)
{
	cv::Mat cut1 = cv::Mat(srcImg, cv::Rect(
		Uma::TrainingCharaNameBound.x * srcImg.size().width,
		Uma::TrainingCharaNameBound.y * srcImg.size().height,
		Uma::TrainingCharaNameBound.width * srcImg.size().width,
		Uma::TrainingCharaNameBound.height * srcImg.size().height
	));
	cv::Mat bin1, bin2, bg;

	// 判定
	cv::inRange(cut1, cv::Scalar(15, 55, 115), cv::Scalar(80, 114, 160), bg);
	double ratio = (double)cv::countNonZero(bg) / bg.size().area();

	if (ratio > 0.04) {
		{
			cv::Mat rsImg, gray;

			cv::resize(cut1, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
			UnsharpMask(rsImg, rsImg, UnsharpRatio);
			cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
			cv::threshold(gray, bin1, 85, 255, cv::THRESH_BINARY);
		}
		{
			cv::Mat cut2 = cv::Mat(srcImg, cv::Rect(
				Uma::TrainingCharaAliasNameBound.x * srcImg.size().width,
				Uma::TrainingCharaAliasNameBound.y * srcImg.size().height,
				Uma::TrainingCharaAliasNameBound.width * srcImg.size().width,
				Uma::TrainingCharaAliasNameBound.height * srcImg.size().height
			));
			cv::Mat rsImg, gray;

			cv::resize(cut2, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
			UnsharpMask(rsImg, rsImg, UnsharpRatio);
			cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
			cv::threshold(gray, bin2, 85, 255, cv::THRESH_BINARY);
		}

		std::wstring alias;
		std::wstring name;

		{
			auto a1 = std::async(std::launch::async, [&] { alias = GetTextFromImage(bin2); });
			auto a2 = std::async(std::launch::async, [&] { name = GetTextFromImage(bin1); });
		}

		if (alias.length() > 0) {
			switch (alias.front()) {
				case '(':
				case '（':
				case '【':
				case '「':
					alias[0] = '［';
					break;
				default:
					alias = L"［" + alias;
					break;
			}

			switch (alias.back()) {
				case ')':
				case '）':
				case '】':
				case '」':
					alias[alias.size() - 1] = '］';
					break;
				default:
					alias += L"］";
					break;
			}
		}

		auto chara = EventLib.CharaEvent.RetrieveName(alias + name);
		if (chara.get() == CurrentCharacter) return nullptr;

		return chara ? chara.get() : nullptr;
	}

	return nullptr;
}

bool Uma::Reload()
{
	CurrentCharacter = nullptr;
	CurrentEvent = nullptr;

	EventLib.Clear();
	EventLib.Load();

	return true;
}

std::shared_ptr<EventSource> Uma::GetCardEvent(const std::vector<std::wstring>& text_list)
{
	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = EventLib.CardEvent.RetrieveTitle(text);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

std::shared_ptr<EventSource> Uma::GetCharaEvent(const std::vector<std::wstring>& text_list)
{
	if (!CurrentCharacter) return nullptr;

	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = EventLib.CharaEvent.RetrieveTitle(text, CurrentCharacter);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

std::shared_ptr<EventSource> Uma::GetScenarioEvent(const std::vector<std::wstring>& text_list)
{
	double best_rate = 0.0;
	std::shared_ptr<EventSource> rvalue;

	for (auto& text : text_list) {
		auto ret = EventLib.ScenarioEvent.RetrieveTitle(text);
		if (ret) {
			double rate = CalcTextMatchRate(text, ret->Name);
			if (rate > best_rate) {
				best_rate = rate;
				rvalue = ret;
			}
		}
	}

	return best_rate > 0.0 ? rvalue : nullptr;
}

std::shared_ptr<EventSource> Uma::RecognizeBottomOption(const cv::Mat& srcImg, BaseData* data, EventRoot* root)
{
	cv::Mat rsImg, gray, bin;
	cv::Mat cut = cv::Mat(srcImg, cv::Rect(
		Uma::BottomChoiseBound.x * srcImg.size().width,
		Uma::BottomChoiseBound.y * srcImg.size().height,
		Uma::BottomChoiseBound.width * srcImg.size().width,
		Uma::BottomChoiseBound.height * srcImg.size().height
	));

	if (!IsBottomOption(cut)) return nullptr;

	ResizeBest(cut, rsImg, srcImg.size().height);
	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::threshold(gray, bin, 90, 255, cv::THRESH_BINARY);

	std::wstring text = Tesseract::Recognize(bin);
	if (!text.empty()) {
		ChangeCollectedText(text);
		auto event = data->RetrieveOption(text, root);
		if (event) return event;
	}

	text = Tesseract::RecognizeAsRaw(bin);
	if (!text.empty()) {
		ChangeCollectedText(text);
		auto event = data->RetrieveOption(text, root);
		if (event) return event;
	}

	text = Tesseract::Recognize(gray);
	if (!text.empty()) {
		ChangeCollectedText(text);
		auto event = data->RetrieveOption(text, root);
		if (event) return event;
	}

	return nullptr;
}



std::vector<std::wstring> Uma::RecognizeCardEventText(const cv::Mat& srcImg, uint64* pHash)
{
	cv::Mat gray, bin, blur;
	cv::Mat bin2;
	cv::Mat rsImg, rsImg2;

	ResizeBest(srcImg, rsImg, ImageSize.height);
	//cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
	rsImg2 = rsImg.clone();
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::bitwise_not(gray, gray);
	bin = Uma::ImageBinarization(rsImg);
	RemoveWhiteSpace(bin, bin);
	cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

	bin2 = Uma::ImageBinarization(rsImg2);
	RemoveWhiteSpace(bin2, bin2);

	gray = cv::Mat(gray, cv::Rect(0, 0, bin.size().width, gray.size().height));

	std::vector<std::wstring> text_list;
	{
		auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
		auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
		auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });

		auto a4 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin2); });
	}

	AppendCollectedText(text_list);
	if (pHash) *pHash = GetImageHash(srcImg);

	return text_list;
}

std::vector<std::wstring> Uma::RecognizeScenarioEventText(const cv::Mat& srcImg, uint64* pHash)
{
	cv::Mat gray, bin, blur;
	cv::Mat bin2;
	cv::Mat rsImg, rsImg2;

	ResizeBest(srcImg, rsImg, ImageSize.height);
	//cv::resize(cut, rsImg, cv::Size(), ResizeRatio, ResizeRatio, cv::INTER_CUBIC);
	rsImg2 = rsImg.clone();
	UnsharpMask(rsImg, rsImg, UnsharpRatio);

	cv::cvtColor(rsImg, gray, cv::COLOR_RGB2GRAY);
	cv::bitwise_not(gray, gray);
	bin = Uma::ImageBinarization(rsImg);
	RemoveWhiteSpace(bin, bin);
	cv::dilate(bin, blur, cv::Mat(2, 2, CV_8U, cv::Scalar(1)), cv::Point(-1, -1), 1);

	bin2 = Uma::ImageBinarization(rsImg2);
	RemoveWhiteSpace(bin2, bin2);

	gray = cv::Mat(gray, cv::Rect(0, 0, bin.size().width, gray.size().height));

	std::vector<std::wstring> text_list;
	{
		auto a1 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin); });
		auto a2 = std::async(std::launch::async, [&] { AsyncFunction(text_list, gray); });
		auto a3 = std::async(std::launch::async, [&] { AsyncFunction(text_list, blur); });

		auto a4 = std::async(std::launch::async, [&] { AsyncFunction(text_list, bin2); });
	}

	AppendCollectedText(text_list);
	if (pHash) *pHash = GetImageHash(srcImg);

	return text_list;
}
