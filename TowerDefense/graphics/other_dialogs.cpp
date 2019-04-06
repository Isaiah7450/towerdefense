// Author: Isaiah Hoffman
// Created: March 11, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <commctrl.h>
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
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<ChallengeLevelDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				const uintptr_t my_clevel = SendMessage(GetDlgItem(hwnd, IDC_CHALLENGE_LEVEL_SELECTOR), CB_GETCURSEL, 0, 0);
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
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Beginner"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Intermediate"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Experienced"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Expert"));
		SendMessage(dlg_clevel, CB_SETCURSEL, 1, 0);
		}
	}

	TerrainEditorNewMapDialog::TerrainEditorNewMapDialog(HWND owner, HINSTANCE h_inst, std::wstring default_name) :
		map_name {default_name} {
		this->create_new_map = DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_TERRAIN_NEW_MAP),
			owner, TerrainEditorNewMapDialog::dialogProc, reinterpret_cast<LPARAM>(this)) == IDOK;
	}

	INT_PTR CALLBACK TerrainEditorNewMapDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<TerrainEditorNewMapDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
				auto& my_dialog_class = *reinterpret_cast<TerrainEditorNewMapDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
				my_dialog_class.map_name.reserve(80);
				my_dialog_class.map_name.at(0) = 80;
				SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_NEW_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(my_dialog_class.map_name.data()));
				my_dialog_class.num_rows = static_cast<int>(GetDlgItemInt(hwnd, IDC_TERRAIN_NEW_MAP_ROWS, nullptr, FALSE));
				my_dialog_class.num_cols = static_cast<int>(GetDlgItemInt(hwnd, IDC_TERRAIN_NEW_MAP_COLS, nullptr, FALSE));
				}
				EndDialog(hwnd, IDOK);
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

	void TerrainEditorNewMapDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_TERRAIN_NEW_MAP_NAME, this->map_name.c_str());
		constexpr const INT min_rows_cols = 10;
		constexpr const INT max_rows_cols = 50;
		INITCOMMONCONTROLSEX icex {};
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_UPDOWN_CLASS;
		InitCommonControlsEx(&icex);
		this->hwnd_rows_select = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING, UPDOWN_CLASS, nullptr,
			WS_CHILDWINDOW | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
			0, 0, 0, 0, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
		SendMessage(this->hwnd_rows_select, UDM_SETBUDDY, reinterpret_cast<WPARAM>(GetDlgItem(hwnd, IDC_TERRAIN_NEW_MAP_ROWS)), 0);
		SendMessage(this->hwnd_rows_select, UDM_SETRANGE32, min_rows_cols, max_rows_cols);
		SendMessage(this->hwnd_rows_select, UDM_SETPOS32, 0, 40);
		InitCommonControlsEx(&icex);
		this->hwnd_cols_select = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING, UPDOWN_CLASS, nullptr,
			WS_CHILDWINDOW | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
			0, 0, 0, 0, hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
		SendMessage(this->hwnd_cols_select, UDM_SETBUDDY, reinterpret_cast<WPARAM>(GetDlgItem(hwnd, IDC_TERRAIN_NEW_MAP_COLS)), 0);
		SendMessage(this->hwnd_cols_select, UDM_SETRANGE32, min_rows_cols, max_rows_cols);
		SendMessage(this->hwnd_cols_select, UDM_SETPOS32, 0, 35);
	}

	TerrainEditorOpenMapDialog::TerrainEditorOpenMapDialog(HWND owner, HINSTANCE h_inst) {
		this->open_map = IDOK == DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_TERRAIN_OPEN_MAP), owner,
			TerrainEditorOpenMapDialog::dialogProc, reinterpret_cast<LPARAM>(this));
	}

	INT_PTR CALLBACK TerrainEditorOpenMapDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<TerrainEditorOpenMapDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
				auto& my_dialog_class = *reinterpret_cast<TerrainEditorOpenMapDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
				my_dialog_class.map_name.reserve(82);
				my_dialog_class.map_name.at(0) = 80;
				SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_OPEN_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(my_dialog_class.map_name.data()));
				}
				EndDialog(hwnd, IDOK);
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

	void TerrainEditorOpenMapDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_TERRAIN_OPEN_MAP_NAME, L"default");
	}
}
