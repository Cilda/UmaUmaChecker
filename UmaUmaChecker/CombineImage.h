#pragma once

class CombineImage
{
public:
	CombineImage();
	~CombineImage();

	void StartCapture();
	void EndCapture();

	bool Combine();
private:
	bool IsCapture;

};

