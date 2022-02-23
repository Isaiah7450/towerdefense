// Author: Isaiah Hoffman
// Created: March 11, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <commctrl.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "./../resource.h"
#include "./other_dialogs.hpp"
#include "./../globals.hpp"
#include "./../game/my_game.hpp"
namespace hoffman_isaiah::winapi {
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
			default:
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

	StartCustomGameDialog::StartCustomGameDialog(HWND owner, HINSTANCE h_inst) {
		this->selected_clevel = static_cast<int>(DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_START_CUSTOM_GAME), owner,
			StartCustomGameDialog::dialogProc, reinterpret_cast<LPARAM>(this)));
	}

	INT_PTR CALLBACK StartCustomGameDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<StartCustomGameDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				auto& my_dialog_class = *reinterpret_cast<StartCustomGameDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
				auto buffer = std::make_unique<wchar_t[]>(81);
				buffer[0] = 80;
				SendMessage(GetDlgItem(hwnd, IDC_CUSTOM_GAME_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(buffer.get()));
				my_dialog_class.map_name = buffer.get();
				const uintptr_t my_clevel = SendMessage(GetDlgItem(hwnd, IDC_CHALLENGE_LEVEL_SELECTOR), CB_GETCURSEL, 0, 0);
				EndDialog(hwnd, ID_CHALLENGE_LEVEL_EASY + my_clevel);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			default:
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void StartCustomGameDialog::initDialog(HWND hwnd) {
		auto dlg_clevel = GetDlgItem(hwnd, IDC_CHALLENGE_LEVEL_SELECTOR);
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Beginner"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Intermediate"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Experienced"));
		SendMessage(dlg_clevel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Expert"));
		SendMessage(dlg_clevel, CB_SETCURSEL, 1, 0);
		}
	}

	GlobalStatsDialog::GlobalStatsDialog(HWND owner, HINSTANCE h_inst, const game::MyGame& my_game) :
		highest_score {my_game.getHiscore()},
		highest_levels {my_game.getHighestLevels()} {
		DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_GLOBAL_STATS), owner,
			GlobalStatsDialog::dialogProc, reinterpret_cast<LPARAM>(this));
	}

	INT_PTR CALLBACK GlobalStatsDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<GlobalStatsDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				EndDialog(hwnd, IDOK);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			default:
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void GlobalStatsDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_GLOBAL_STATS_HISCORE, std::to_wstring(this->getHiscore()).c_str());
		SetDlgItemText(hwnd, IDC_GLOBAL_STATS_EASY, std::to_wstring(this->getHighestLevels().at(ID_CHALLENGE_LEVEL_EASY)).c_str());
		SetDlgItemText(hwnd, IDC_GLOBAL_STATS_NORMAL, std::to_wstring(this->getHighestLevels().at(ID_CHALLENGE_LEVEL_NORMAL)).c_str());
		SetDlgItemText(hwnd, IDC_GLOBAL_STATS_HARD, std::to_wstring(this->getHighestLevels().at(ID_CHALLENGE_LEVEL_HARD)).c_str());
		SetDlgItemText(hwnd, IDC_GLOBAL_STATS_EXPERT, std::to_wstring(this->getHighestLevels().at(ID_CHALLENGE_LEVEL_EXPERT)).c_str());
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
				auto buffer = std::make_unique<wchar_t[]>(81);
				buffer[0] = 80;
				SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(buffer.get()));
				my_dialog_class.map_name = buffer.get();
				my_dialog_class.num_rows = static_cast<int>(GetDlgItemInt(hwnd, IDC_TERRAIN_NEW_MAP_ROWS, nullptr, FALSE));
				my_dialog_class.num_cols = static_cast<int>(GetDlgItemInt(hwnd, IDC_TERRAIN_NEW_MAP_COLS, nullptr, FALSE));
				}
				EndDialog(hwnd, IDOK);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			default:
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void TerrainEditorNewMapDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_TERRAIN_MAP_NAME, this->map_name.c_str());
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
				auto buffer = std::make_unique<wchar_t[]>(81);
				buffer[0] = 80;
				SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(buffer.get()));
				my_dialog_class.map_name = buffer.get();
				}
				EndDialog(hwnd, IDOK);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			default:
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void TerrainEditorOpenMapDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_TERRAIN_MAP_NAME, L"default");
	}

	TerrainEditorSaveMapAsDialog::TerrainEditorSaveMapAsDialog(HWND owner, HINSTANCE h_inst, std::wstring default_name) :
		map_name {default_name} {
		this->save_map = IDOK == static_cast<int>(DialogBoxParam(h_inst, MAKEINTRESOURCE(IDD_TERRAIN_SAVE_MAP_AS), owner,
			TerrainEditorSaveMapAsDialog::dialogProc, reinterpret_cast<LPARAM>(this)));
	}

	INT_PTR CALLBACK TerrainEditorSaveMapAsDialog::dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			const auto my_dialog_class = reinterpret_cast<TerrainEditorSaveMapAsDialog*>(lparam);
			my_dialog_class->initDialog(hwnd);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
			{
				[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
				auto& my_dialog_class = *reinterpret_cast<TerrainEditorSaveMapAsDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
				auto buffer = std::make_unique<wchar_t[]>(81);
				buffer[0] = 80;
				SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_MAP_NAME), EM_GETLINE, 0, reinterpret_cast<LPARAM>(buffer.get()));
				my_dialog_class.map_name = buffer.get();
				my_dialog_class.show_overwrite_confirm = BST_CHECKED
					== SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_SAVE_MAP_AS_SHOW_CONFIRM), BM_GETCHECK, 0, 0);
				}
				EndDialog(hwnd, IDOK);
				break;
			}
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				break;
			default:
				break;
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	void TerrainEditorSaveMapAsDialog::initDialog(HWND hwnd) {
		SetDlgItemText(hwnd, IDC_TERRAIN_MAP_NAME, this->getName().c_str());
		SendMessage(GetDlgItem(hwnd, IDC_TERRAIN_SAVE_MAP_AS_SHOW_CONFIRM), BM_SETCHECK, BST_CHECKED, 0);
	}
}
