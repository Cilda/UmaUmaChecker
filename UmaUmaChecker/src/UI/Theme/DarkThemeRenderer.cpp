#include "DarkThemeRenderer.h"

#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/msw/dc.h>
#include <wx/msw/uxtheme.h>
#include <wx/combobox.h>

#include <vssym32.h>


wxIMPLEMENT_DYNAMIC_CLASS(DarkThemeRenderer, BaseRenderer);

void DarkThemeRenderer::OnInit()
{
}

void DarkThemeRenderer::OnRelease()
{
}

void DarkThemeRenderer::InitWindow(wxWindow* window)
{

}

void DarkThemeRenderer::InitButton(wxWindow* ctrl)
{
	SetWindowTheme(ctrl->GetHWND(), L"DarkMode_Explorer", NULL);
}

void DarkThemeRenderer::InitComboBox(wxWindow* ctrl)
{
	COMBOBOXINFO cbi;
	cbi.cbSize = sizeof(COMBOBOXINFO);

	GetComboBoxInfo(ctrl->GetHWND(), &cbi);
	ctrl->SetBackgroundColour(wxColor(56, 56, 56));
	ctrl->SetForegroundColour(wxColor(255, 255, 255));

	SetWindowTheme(cbi.hwndList, L"DarkMode_Explorer", NULL);
	SetWindowTheme(ctrl->GetHWND(), L"DarkMode_CFD", NULL);
	//SetWindowTheme(cbi.hwndItem, L"DarkMode_CFD", NULL);
}

void DarkThemeRenderer::InitEdit(wxWindow* ctrl)
{
	SetWindowTheme(ctrl->GetHWND(), L"DarkMode_CFD", NULL);
	if (!ctrl->IsTransparentBackgroundSupported()) ctrl->SetBackgroundColour(wxColor(56, 56, 56));
	ctrl->SetForegroundColour(wxColor(255, 255, 255));
}

void DarkThemeRenderer::InitListBox(wxWindow* ctrl)
{
	SetWindowTheme(ctrl->GetHWND(), L"DarkMode_Explorer", NULL);
}

void DarkThemeRenderer::InitCommonControl(wxWindow* ctrl)
{
	if (!ctrl->IsTransparentBackgroundSupported()) ctrl->SetBackgroundColour(wxColor(56, 56, 56));
	ctrl->SetForegroundColour(wxColor(255, 255, 255));
}

void DarkThemeRenderer::DrawPushButton(wxWindow* control, wxDC& dc, const wxRect& rect, int OldState, int State)
{
	HDC hdc = dc.GetHDC();
	wxUxThemeHandle theme(control, L"DarkMode_Explorer::Button");
	RECT rc = { 0, 0, rect.width, rect.height };

	if (!BufferedPaintRenderAnimation(control->GetHWND(), hdc)) {
		if (true) {
			BP_ANIMATIONPARAMS params = { sizeof(BP_ANIMATIONPARAMS) };
			params.style = BPAS_LINEAR;

			GetThemeTransitionDuration(theme, BP_PUSHBUTTON, OldState, State, TMT_TRANSITIONDURATIONS, &params.dwDuration);

			HDC hdcFrom, hdcTo;
			HANIMATIONBUFFER hab = BeginBufferedAnimation(control->GetHWND(), hdc, &rc, BPBF_COMPATIBLEBITMAP, NULL, &params, &hdcFrom, &hdcTo);

			if (hab) {
				if (hdcFrom) {
					wxDCTemp dctemp(hdcFrom);
					dctemp.SetFont(dc.GetFont());
					DrawButton(control->GetHWND(), dctemp, control->GetLabel(), rect, OldState, theme);
				}
				if (hdcTo) {
					wxDCTemp dctemp(hdcTo);
					dctemp.SetFont(dc.GetFont());
					DrawButton(control->GetHWND(), dctemp, control->GetLabel(), rect, State, theme);
				}

				EndBufferedAnimation(hab, TRUE);
			}
			else {
				DrawButton(control->GetHWND(), dc, control->GetLabel(), rect, State, theme);
			}
		}
	}
}

void DarkThemeRenderer::DrawButton(WXHWND hWnd, wxDC& dc, const wxString& label, const wxRect& rect, int State, wxUxThemeHandle& theme)
{
	HDC hdc = dc.GetHDC();
	RECT rc = { 0, 0, rect.width, rect.height };

	if (IsThemeBackgroundPartiallyTransparent(theme, BP_PUSHBUTTON, State)) {
		DrawThemeParentBackground(hWnd, hdc, &rc);
	}
	DrawThemeBackground(theme, hdc, BP_PUSHBUTTON, State, &rc, NULL);

	COLORREF color = 0;
	HRESULT hResult = GetThemeColor(theme, BP_PUSHBUTTON, State, TMT_TEXTCOLOR, &color);
	if (hResult != S_OK) {
		GetThemeColor(theme, BP_PUSHBUTTON, 0, TMT_TEXTCOLOR, &color);
	}

	dc.SetTextForeground(wxColor(color));
	dc.DrawLabel(label, rect, wxALIGN_CENTER);
}
