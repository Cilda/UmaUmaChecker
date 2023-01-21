#pragma once

#include <wx/window.h>
#include <wx/brush.h>
#include <wx/button.h>
#include <wx/dcclient.h>
#include <vssym32.h>
#include <dwmapi.h>

#include "Theme/ThemeManager.h"
#include "Theme/DarkThemeRenderer.h"


template<class WindowBase>
class ThemedWindowWrapper : public WindowBase
{
public:
	template<typename... Args>
	ThemedWindowWrapper(Args&&... args) : WindowBase(std::forward<Args>(args)...)
	{
		ThemeManager::GetInstance()->InitWindow(this);
		this->SetBackgroundColour(ThemeManager::GetInstance()->GetBackgroundColor());
		SetDarkTitleBar();
		observer.Subscribe(&ThemedWindowWrapper::OnThemeChanged, this);
	}

private:
	void OnThemeChanged()
	{
		ThemeManager::GetInstance()->InitWindow(this);
		this->SetBackgroundColour(ThemeManager::GetInstance()->GetBackgroundColor());
		SetDarkTitleBar();
		this->Refresh();
	}

	void SetDarkTitleBar()
	{
		BOOL value = ThemeManager::GetInstance()->IsKindOf(wxCLASSINFO(DarkThemeRenderer));
		DwmSetWindowAttribute(this->GetHWND(), DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	}

private:
	ThemeChangedNotify observer;
};

template<class WindowBase>
class ThemedWrapper final : public WindowBase
{
public:
	template<typename... Args>
	ThemedWrapper(Args&&... args) : WindowBase(std::forward<Args>(args)...)
	{
		ThemeManager::GetInstance()->InitCommonControl(this);

		observer.Subscribe(&ThemedWrapper::OnThemeChanged, this);
	}

protected:
	void OnThemeChanged()
	{
		if (ThemeManager::GetInstance()->IsSystemRender()) {
			ThemeManager::GetInstance()->InitCommonControl(this);
		}
	}

private:
	ThemeChangedNotify observer;
};

template<class ButtonBase>
class ThemedButtonWrapper final : public ButtonBase
{
public:
	template<typename... Args>
	ThemedButtonWrapper(Args&&... args) : ButtonBase(std::forward<Args>(args)...)
	{
		OldState = 0;
		bPushed = false;

		HookEvent();
		ThemeManager::GetInstance()->InitButton(this);

		observer.Subscribe(&ThemedButtonWrapper::OnThemeChanged, this);
	}

	~ThemedButtonWrapper()
	{
	}

	virtual WXHBRUSH MSWControlColor(WXHDC pDC, WXHWND hWnd)
	{
		wxBrush* brush = wxTheBrushList->FindOrCreateBrush(ThemeManager::GetInstance()->GetBackgroundColor());
		return (WXHBRUSH)brush->GetResourceHandle();
	}

private:
	void HookEvent()
	{
		if (ThemeManager::GetInstance()->IsSystemRender()) {
			this->Unbind(wxEVT_PAINT, &ThemedButtonWrapper::OnPaint, this);
			this->Unbind(wxEVT_LEFT_DOWN, &ThemedButtonWrapper::OnLeftDown, this);
			this->Unbind(wxEVT_LEFT_UP, &ThemedButtonWrapper::OnLeftUp, this);
		}
		else {
			this->Bind(wxEVT_PAINT, &ThemedButtonWrapper::OnPaint, this);
			this->Bind(wxEVT_LEFT_DOWN, &ThemedButtonWrapper::OnLeftDown, this);
			this->Bind(wxEVT_LEFT_UP, &ThemedButtonWrapper::OnLeftUp, this);
		}
	}

	void OnLeftDown(wxMouseEvent& event)
	{
		bPushed = true;
		event.Skip();
	}

	void OnLeftUp(wxMouseEvent& event)
	{
		bPushed = false;
		event.Skip();
	}

	void OnErase(wxEraseEvent& event)
	{
		//this->GetParent()->DoEraseBackground(event.GetDC());
		event.Skip();
	}

	void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
		WXHDC hdc = dc.GetHDC();
		wxSize size = dc.GetSize();
		wxRect rect(0, 0, size.x, size.y);
		int State = PBS_NORMAL;

		if (bPushed && this->IsMouseInWindow()) State = PBS_PRESSED;
		else if (bPushed || this->IsMouseInWindow()) State = PBS_HOT;
		else if (!this->IsThisEnabled()) State = PBS_DISABLED;
		else if (this->HasFocus()) State = PBS_DEFAULTED;

		ThemeManager::GetInstance()->DrawPushButton(this, dc, rect, OldState, State);

		OldState = State;
	}

protected:
	void OnThemeChanged()
	{
		if (ThemeManager::GetInstance()->IsSystemRender()) {
			ThemeManager::GetInstance()->InitButton(this);
		}

		HookEvent();
	}

private:
	int OldState;
	bool bPushed;
	ThemeChangedNotify observer;
};

template<class ComboBoxBase>
class ThemedComboBoxWrapper final : public ComboBoxBase
{
public:
	template<typename... Args>
	ThemedComboBoxWrapper(Args&&... args) : ComboBoxBase(std::forward<Args>(args)...)
	{
		ThemeManager::GetInstance()->InitComboBox(this);
		observer.Subscribe(&ThemedComboBoxWrapper::OnThemeChanged, this);
	}

	~ThemedComboBoxWrapper()
	{
	}

private:
	void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
	}

protected:
	void OnThemeChanged()
	{
		if (ThemeManager::GetInstance()->IsSystemRender()) {
			ThemeManager::GetInstance()->InitComboBox(this);
		}
	}

private:
	ThemeChangedNotify observer;
};

template<class TextCtrlBase>
class ThemedEditWrapper final : public TextCtrlBase
{
public:
	template<typename... Args>
	ThemedEditWrapper(Args&&... args) : TextCtrlBase(std::forward<Args>(args)...)
	{
		ThemeManager::GetInstance()->InitEdit(this);
		observer.Subscribe(&ThemedEditWrapper::OnThemeChanged, this);
	}

	~ThemedEditWrapper()
	{
	}

private:
	void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
	}

protected:
	void OnThemeChanged()
	{
		if (ThemeManager::GetInstance()->IsSystemRender()) {
			ThemeManager::GetInstance()->InitEdit(this);
		}

		this->Refresh();
	}

private:
	ThemeChangedNotify observer;
};
