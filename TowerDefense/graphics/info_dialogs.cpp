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

	EnemyInfoDialog::EnemyInfoDialog(HWND owner, HINSTANCE h_inst, const game::EnemyType& etype) :
		InfoDialogBase {h_inst, etype} {
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
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
			<< my_etype.getExtraRating();
		this->buff_rating_string = my_stream.str();
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
			<< my_etype.getRating();
		this->rating_string = my_stream.str();
		my_stream.str(L"");
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
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_BUFF_RATING, this->buff_rating_string.c_str());
		SetDlgItemText(hwnd, IDC_INFO_ENEMY_RATING, this->rating_string.c_str());
	}

	TowerInfoDialog::TowerInfoDialog(HWND owner, HINSTANCE h_inst, const game::TowerType& ttype) :
		InfoDialogBase {h_inst, ttype} {
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_TOWER),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
	}

	void TowerInfoDialog::initDialog(HWND hwnd) {
		const auto& my_ttype = dynamic_cast<const game::TowerType&>(this->getType());
		SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_METHOD, my_ttype.getFiringMethod().getReferenceName().c_str());
		SetDlgItemText(hwnd, IDC_INFO_TOWER_TARGETING_STRATEGY, my_ttype.getTargetingStrategy().getReferenceName().c_str());
		std::wstringstream my_stream {};
		// Add ammo types.
		const auto hdlg_ammo = GetDlgItem(hwnd, IDC_INFO_TOWER_AMMO_TYPES);
		for (const auto& st_pair : my_ttype.getShotTypes()) {
			my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
				<< (st_pair.second * 100) << L"%";
			const std::wstring ammo_string = st_pair.first->getName() + L": " + my_stream.str();
			my_stream.str(L"");
			SendMessage(hdlg_ammo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ammo_string.c_str()));
		}
		SendMessage(hdlg_ammo, CB_SETCURSEL, 0, 0);
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_ttype.getFiringSpeed() << L" / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_SPEED, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_ttype.getFiringRange() << L" cs";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_RANGE, my_stream.str().c_str());
		my_stream.str(L"");
		SetDlgItemText(hwnd, IDC_INFO_TOWER_VOLLEY_SHOTS, std::to_wstring(my_ttype.getVolleyShots()).c_str());
		my_stream << my_ttype.getReloadDelay() << L" ms";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RELOAD_DELAY, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_ttype.getAverageDamagePerShot();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_EXPECTED_SHOT_DAMAGE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_ttype.getAverageShotRating();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_AVG_SHOT_RATING, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_ttype.getRateOfFire() << L" / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RATE_OF_FIRE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_ttype.getExpectedDPS() << L" dmg / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_EXPECTED_DPS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << L"$" << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_ttype.getCost();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_COST, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
			<< my_ttype.getRating();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RATING, my_stream.str().c_str());
		my_stream.str(L"");
	}
}

