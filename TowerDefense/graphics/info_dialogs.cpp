// Author: Isaiah Hoffman
// Created: March 5, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include "./../resource.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "./info_dialogs.hpp"
#include "./../globals.hpp"
#include "./../game/enemy_type.hpp"
#include "./../game/shot_types.hpp"
#include "./../game/tower_types.hpp"
using namespace std::literals::string_literals;
namespace hoffman::isaiah::winapi {
	InfoDialogBase::InfoDialogBase(HINSTANCE h_inst, const game::GameObjectType& object_type) :
		h_instance {h_inst},
		my_type {object_type} {
	}

	INT_PTR CALLBACK InfoDialogBase::infoDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_INITDIALOG:
		{
			const auto my_dialog_class = reinterpret_cast<InfoDialogBase*>(lparam);
			// Set common stuff.
			SetDlgItemText(hwnd, IDC_INFO_BASE_NAME, (my_dialog_class->my_type.getName()).c_str());
			SetDlgItemText(hwnd, IDC_INFO_BASE_DESC, (my_dialog_class->my_type.getDesc()).c_str());
			my_dialog_class->initDialog(hwnd);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
			case IDOK:
				EndDialog(hwnd, IDOK);
				break;
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

	EnemyInfoDialog::EnemyInfoDialog(HWND owner, HINSTANCE h_inst, const game::EnemyType& e) :
		InfoDialogBase {h_inst, e} {
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_ENEMY),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
	}

	void EnemyInfoDialog::initDialog(HWND hwnd) {
		const auto& my_etype = dynamic_cast<const game::EnemyType&>(this->getType());
		this->hp_string = std::to_wstring(static_cast<int>(my_etype.getBaseHP()));
		this->ahp_string = std::to_wstring(static_cast<int>(my_etype.getBaseArmorHP()));
		std::wstringstream my_stream {};
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_etype.getArmorReduce() * 100) << L"%";
		this->ar_string = my_stream.str();
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_etype.getPainTolerance() * 100) << L"%";
		this->pt_string = my_stream.str();
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_etype.getBaseWalkingSpeed() << L" cs / s";
		this->walk_string = my_stream.str();
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_etype.getBaseRunningSpeed() << L" cs / s";
		this->run_string = my_stream.str();
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_etype.getBaseInjuredSpeed() << L" cs / s";
		this->injured_string = my_stream.str();
		my_stream.str(L"");
		this->fly_string = my_etype.isFlying() ? L"Yes" : L"No";
		this->dmg_string = std::to_wstring(my_etype.getDamage());
		this->num_buffs_string = std::to_wstring(my_etype.getBuffTypesCount());
		this->strat_string = *my_etype.getDefaultStrategy();
		this->diag_string = my_etype.canMoveDiagonally() ? L"Yes" : L"No";
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_HEALTH, this->hp_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_ARMOR_HP, this->ahp_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_ARMOR_REDUCE, this->ar_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_PAIN_TOLERANCE, this->pt_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_WALK_SPEED, this->walk_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_RUN_SPEED, this->run_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_INJURED_SPEED, this->injured_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_IS_FLYING, this->fly_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_DAMAGE, this->dmg_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_NUM_BUFFS, this->num_buffs_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_STRATEGY, this->strat_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_MOVE_DIAGONAL, this->diag_string.c_str());
	}
}

