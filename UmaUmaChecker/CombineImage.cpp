#include "CombineImage.h"

CombineImage::CombineImage() : IsCapture(false)
{
}

CombineImage::~CombineImage()
{
}

void CombineImage::StartCapture()
{
	if (IsCapture) return;

	IsCapture = true;
}

void CombineImage::EndCapture()
{
	if (!IsCapture) return;

	IsCapture = false;
}

bool CombineImage::Combine()
{
	return false;
}
