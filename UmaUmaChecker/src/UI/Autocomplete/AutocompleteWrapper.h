#pragma once

#include <vector>
#include <string>
#include <utility>

#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/thread.h>

#include "UI/wxComboBoxPopup.h"

template<class TextCtrlBase>
class AutocompleteWrapper : public TextCtrlBase, wxThreadHelper
{
public:
	template<typename... Args>
	AutocompleteWrapper(Args&& ...args) : TextCtrlBase(std::forward<Args>(args)...), wxThreadHelper(wxTHREAD_JOINABLE)
	{
		this->Bind(wxEVT_COMMAND_TEXT_UPDATED, &AutocompleteWrapper::OnTextUpdate, this);
		this->Bind(wxEVT_KEY_DOWN, &AutocompleteWrapper::OnKeyDown, this);
		this->Bind(wxEVT_THREAD, &AutocompleteWrapper::OnThreadUpdate, this);
	}

	void SetAutoCompleteItems(const std::vector<std::wstring>& items)
	{
		this->items = items;
	}

protected:
	wxThread::ExitCode Entry() override
	{
		wxArrayString result;

		int index = 0;

		while (!GetThread()->TestDestroy() && index < items.size()) {
			auto& item = items[index];

			if (item.find(containString) != std::wstring::npos) {
				result.push_back(item);
			}

			index++;
		}

		wxThreadEvent* event = new wxThreadEvent();

		event->SetPayload(result);
		wxQueueEvent(this, event);

		return (wxThread::ExitCode)0;
	}

private:
	wxComboBoxPopup CreatePopup()
	{
		if (!popup) {
			popup = new wxComboBoxPopup(this);
			popup->Bind(wxEVT_HIDE, &AutocompleteWrapper::OnSelectedItem, this);
		}

		wxPoint pos = this->ClientToScreen(wxPoint(-2, -2));
		popup->Position(pos, wxSize(0, this->GetSize().y));
		popup->SetSize(wxSize(this->GetSize().x, this->FromDIP(100)));

		return popup;
	}

	void StartAutoComplete(const wxString& contains)
	{
		if (items.empty()) {
			return;
		}

		wxThread* thread = GetThread();
		if (thread && thread->IsRunning()) {
			thread->Delete();
		}

		if (CreateThread(m_kind) != wxTHREAD_NO_ERROR) {
			return;
		}

		if (GetThread()->Run() != wxTHREAD_NO_ERROR) {
			return;
		}

		containString = contains;
	}

private:
	void OnTextUpdate(wxCommandEvent& event)
	{
		wxString text = event.GetString();

		if (!text.empty()) {
			StartAutoComplete(text);
		}
		else if (popup) {
			popup->Destroy();
			popup = nullptr;
		}
	}

	void OnKeyDown(wxKeyEvent& event)
	{
		if (!popup) {
			event.Skip();
			return;
		}
		else if (popup->IsShown()) {
			if (event.m_keyCode == WXK_UP) {
				int index = popup->m_listBox->GetSelection();
				if (index != wxNOT_FOUND) {
					if (index >= 1) {
						popup->m_listBox->SetSelection(index - 1);
					}
				}
				else {
					if (popup->m_listBox->GetCount() > 0) popup->m_listBox->SetSelection(0);
				}
				return;
			}
			else if (event.m_keyCode == WXK_DOWN) {
				int index = popup->m_listBox->GetSelection();
				if (index != wxNOT_FOUND) {
					if (index + 1 < popup->m_listBox->GetCount()) {
						popup->m_listBox->SetSelection(index + 1);
					}
				}
				else {
					if (popup->m_listBox->GetCount() > 0) popup->m_listBox->SetSelection(0);
				}
				return;
			}
			else if (event.m_keyCode == WXK_RETURN) {
				int index = popup->m_listBox->GetSelection();
				if (index != wxNOT_FOUND) {
					wxString value = popup->m_listBox->GetStringSelection();

					this->ChangeValue(value);
					//this->SetStringSelection(value.ToStdWstring());
					//umaMgr->SetTrainingCharacter(value.ToStdWstring());
					popup->Dismiss();
					return;
				}
			}
		}

		event.Skip();
	}

	void OnSelectedItem(wxCommandEvent& event)
	{
		this->ChangeValue(event.GetString());
	}

	void OnThreadUpdate(wxThreadEvent& event)
	{
		auto list = event.GetPayload<wxArrayString>();

		if (list.size() > 0) {
			CreatePopup();

			popup->ClearList();

			for (auto& item : list) {
				popup->AddString(item);
			}

			popup->Popup();
		}
		else if (popup) {
			popup->Destroy();
			popup = nullptr;
		}
	}

private:
	wxComboBoxPopup* popup;

	wxString containString;
	std::vector<std::wstring> items;
};

