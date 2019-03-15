// Author: Isaiah Hoffman
// Created: March 11, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include "./../resource.h"
#include "./other_dialogs.hpp"
#include "./../globals.hpp"
namespace hoffman::isaiah::winapi {
	ChallengeLevelDialog::ChallengeLevelDialog(HWND owner, HINSTANCE h_inst) {
		this->selected_clevel = static_cast<int>(DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_CHALLENGE_LEVEL),
			owner, ChallengeLevelDialog::dialogProc, reinterpret_cast<LPARAM>(this)));
	}

	INT_PTR CALLBACK ChallengeLevelDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			const auto my_dialog_class = reinterpret_cast<ChallengeLevelDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				const int my_clevel = static_cast<int>(SendMessage(GetDlgItem(hwnd, IDC_CHALLENGE_LEVEL_SELECTOR), CB_GETCURSEL, 0, 0));
				EndDialog(hwnd, ID_CHALLENGE_LEVEL_EASY + my_clevel);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void ChallengeLevelDialog::initDialog(HWND hwnd) {
		auto dlg_clevel = GetDlgItem(hwnd, IDC_CHALLENGE_LEVEL_SELECTOR);
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Beginner"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Intermediate"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Experienced"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Expert"));
		SendMessage(dlg_clevel, CB_SETCURSEL, 1, 0);
	}
}
