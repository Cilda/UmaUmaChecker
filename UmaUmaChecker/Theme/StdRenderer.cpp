#include "StdRenderer.h"

#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/msw/dc.h>
#include <wx/msw/uxtheme.h>

#include <vssym32.h>

wxIMPLEMENT_DYNAMIC_CLASS(StdRenderer, BaseRenderer);

void StdRenderer::OnInit()
{
}

void StdRenderer::OnRelease()
{
}

void StdRenderer::InitWindow(wxWindow* window)
{
}

void StdRenderer::InitButton(wxWindow* ctrl)
{
	SetWindowTheme(ctrl->GetHWND(), L"Button", NULL);
}

void StdRenderer::InitComboBox(wxWindow* ctrl)
{
	COMBOBOXINFO cbi;
	cbi.cbSize = sizeof(COMBOBOXINFO);

	GetComboBoxInfo(ctrl->GetHWND(), &cbi);
	ctrl->SetBackgroundColour(wxColor(255, 255, 255));
	ctrl->SetForegroundColour(wxColor(0, 0, 0));

	SetWindowTheme(cbi.hwndList, L"Explorer", NULL);
	SetWindowTheme(ctrl->GetHWND(), L"Combobox", NULL);
	//SetWindowTheme(cbi.hwndItem, L"DarkMode_CFD", NULL);
}

void StdRenderer::InitEdit(wxWindow* ctrl)
{
	SetWindowTheme(ctrl->GetHWND(), L"Edit", NULL);
	if (!ctrl->IsTransparentBackgroundSupported()) ctrl->SetBackgroundColour(wxColor(255, 255, 255));
	ctrl->SetForegroundColour(wxColor(0, 0, 0));
}

void StdRenderer::InitCommonControl(wxWindow* ctrl)
{
	if (!ctrl->IsTransparentBackgroundSupported()) ctrl->SetBackgroundColour(wxColor(255, 255, 255));
	ctrl->SetForegroundColour(wxColor(0, 0, 0));
}

void StdRenderer::DrawPushButton(wxWindow* control, wxDC& dc, const wxRect& rect, int OldState, int State)
{
	HDC hdc = dc.GetHDC();
	wxUxThemeHandle theme(control, L"Button");
	RECT rc = { 0, 0, rect.width, rect.height };

	if (!BufferedPaintRenderAnimation(control->GetHWND(), hdc)) {
		if (true) {
			BP_ANIMATIONPARAMS params = { sizeof(BP_ANIMATIONPARAMS) };
			params.style = BPAS_LINEAR;
			params.dwDuration = 0;

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

void StdRenderer::DrawButton(WXHWND hWnd, wxDC& dc, const wxString& label, const wxRect& rect, int State, wxUxThemeHandle& theme)
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
		hResult = GetThemeColor(theme, BP_PUSHBUTTON, 0, TMT_TEXTCOLOR, &color);
	}

	dc.SetTextForeground(wxColor(color));
	dc.DrawLabel(label, rect, wxALIGN_CENTER);
}
