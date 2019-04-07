// Author: Isaiah Hoffman
// Created: March 5, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <commctrl.h>
#include "./../resource.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "./info_dialogs.hpp"
#include "./../globals.hpp"
#include "./../game/enemy_type.hpp"
#include "./../game/game_formulas.hpp"
#include "./../game/my_game.hpp"
#include "./../game/shot_types.hpp"
#include "./../game/status_effects.hpp"
#include "./../game/tower_types.hpp"
#include "./../game/tower.hpp"
using namespace std::literals::string_literals;
namespace hoffman::isaiah::winapi {
	InfoDialogBase::InfoDialogBase(HINSTANCE h_inst, const game::GameObjectType& object_type) :
		h_instance {h_inst},
		my_type {object_type} {
	}

	INT_PTR CALLBACK InfoDialogBase::infoDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret cast.
		switch (msg) {
		case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			const auto my_dialog_class = reinterpret_cast<InfoDialogBase*>(lparam);
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
				// Shot-specific code:
			case IDC_INFO_SHOT_BASE_VIEW_TYPE_INFO:
			{
				const auto my_dialog_class = reinterpret_cast<const InfoDialogBase*>(
					GetWindowLongPtr(hwnd, GWLP_USERDATA));
				const auto& my_stype = dynamic_cast<const game::ShotBaseType&>(my_dialog_class->getType());
				switch (my_stype.getType()) {
				case game::ShotTypes::Standard:
					break;
				case game::ShotTypes::Stun:
				{
					const ShotStunInfoDialog my_stun_dialog {hwnd, my_dialog_class->getApplicationHandle(), my_stype};
					break;
				}
				case game::ShotTypes::Slow:
				{
					const ShotSlowInfoDialog my_slow_dialog {hwnd, my_dialog_class->getApplicationHandle(), my_stype};
					break;
				}
				case game::ShotTypes::DoT:
				{
					const ShotDoTInfoDialog my_dot_dialog {hwnd, my_dialog_class->getApplicationHandle(), my_stype};
					break;
				}
				case game::ShotTypes::Sentinel_DO_NOT_USE:
					MessageBox(hwnd, L"Invalid code branch reached. Please report to developer.",
						L"Error!", MB_OK);
					break;
				default:
					MessageBox(hwnd, L"Unhandled shot type. Please report to developer (include"
						L" the value of the type field in your report).",
						L"Error!", MB_OK | MB_ICONWARNING);
				}
				break;
			}
			// Tower-specific code:
			case IDC_INFO_TOWER_PLACED_UPGRADE_A:
			case IDC_INFO_TOWER_PLACED_UPGRADE_B:
			{
				auto my_dialog_class = reinterpret_cast<TowerPlacedInfoDialog*>(
					GetWindowLongPtr(hwnd, GWLP_USERDATA));
				const auto chosen_option = LOWORD(wparam) == IDC_INFO_TOWER_PLACED_UPGRADE_A
					? game::TowerUpgradeOption::One : game::TowerUpgradeOption::Two;
				const auto old_level = my_dialog_class->getTower().getLevel();
				TowerUpgradeInfoDialog my_upgrade_dialog {hwnd, my_dialog_class->getApplicationHandle(), my_dialog_class->getTower(),
					chosen_option};
				if (my_dialog_class->getTower().getLevel() > old_level) {
					EndDialog(hwnd, IDOK);
				}
				break;
			}
			case IDC_INFO_TOWER_PLACED_SELL:
			{
				auto my_dialog_class = reinterpret_cast<TowerPlacedInfoDialog*>(
					GetWindowLongPtr(hwnd, GWLP_USERDATA));
				my_dialog_class->doSell();
				EndDialog(hwnd, IDOK);
				break;
			}
			case IDC_INFO_TOWER_UPGRADE_DO_UPGRADE:
			{
				auto my_dialog_class = reinterpret_cast<TowerUpgradeInfoDialog*>(
					GetWindowLongPtr(hwnd, GWLP_USERDATA));
				my_dialog_class->doUpgrade();
				EndDialog(hwnd, IDOK);
				break;
			}
			}
			break;
		case WM_CTLCOLORSTATIC:
		{
			const auto my_dialog_class = reinterpret_cast<const InfoDialogBase*>(
				GetWindowLongPtr(hwnd, GWLP_USERDATA));
			HDC hdc_static = reinterpret_cast<HDC>(wparam);
			static const HBRUSH background_brush = GetSysColorBrush(CTLCOLOR_DLG);
			for (const auto& control_pair : my_dialog_class->getControlColorMap()) {
				if (GetDlgCtrlID(reinterpret_cast<HWND>(lparam)) == control_pair.first) {
					SetTextColor(hdc_static, control_pair.second);
					break;
				}
			}
			SetBkMode(hdc_static, TRANSPARENT);
			return reinterpret_cast<INT_PTR>(background_brush);
			break;
		}
		default:
			return FALSE;
		}
		return TRUE;
		}
	}

	void InfoDialogBase::setCommonControls(HWND hwnd) const noexcept {
		SetDlgItemText(hwnd, IDC_INFO_BASE_NAME, (this->getType().getName()).c_str());
		SetDlgItemText(hwnd, IDC_INFO_BASE_DESC, (this->getType().getDesc()).c_str());
	}

	EnemyInfoDialog::EnemyInfoDialog(HWND owner, HINSTANCE h_inst, const game::EnemyType& etype) :
		InfoDialogBase {h_inst, etype} {
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_ENEMY),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
	}

	void EnemyInfoDialog::initDialog(HWND hwnd) {
		this->setCommonControls(hwnd);
		const auto& my_etype = dynamic_cast<const game::EnemyType&>(this->getType());
		this->hp_string = std::to_wstring(static_cast<int>(my_etype.getBaseHealth()));
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

	ShotBaseInfoDialog::ShotBaseInfoDialog(HWND owner, HINSTANCE h_inst, const game::ShotBaseType& stype) :
		InfoDialogBase {h_inst, stype} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_SHOT_BASE),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void ShotBaseInfoDialog::initDialog(HWND hwnd) {
		this->setCommonControls(hwnd);
		const auto& my_stype = dynamic_cast<const game::ShotBaseType&>(this->getType());
		std::wstringstream my_stream {};
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getDamage();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_DAMAGE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getPiercing() * 100.00) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_PIERCING, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_stype.getSpeed();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_MOVE_SPEED, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getSplashDamage();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_SPLASH_DAMAGE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getImpactRadius() << L" cs";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_IMPACT_RADIUS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getGroundMultiplier() * 100.00) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_GROUND_MULTI, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getAirMultiplier() * 100.00) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_AIR_MULTI, my_stream.str().c_str());
		my_stream.str(L"");
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_TYPE, (*my_stype.getType()).c_str());
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_APPLY_ON_SPLASH,
			(my_stype.isSplashEffectType() ? L"Yes" : L"No"));
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (1.0 + my_stype.getAverageExtraTargets());
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_NUM_TARGETS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getBaseRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_RAW_DAMAGE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getExtraRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_EXTRA_RATING, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_RATING, my_stream.str().c_str());
		my_stream.str(L"");
		if (my_stype.getType() == game::ShotTypes::Standard) {
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_SHOT_BASE_VIEW_TYPE_INFO), FALSE);
		}
	}

	TowerInfoDialog::TowerInfoDialog(HWND owner, HINSTANCE h_inst, const game::TowerType& ttype) :
		InfoDialogBase {h_inst, ttype} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpet cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_TOWER),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void TowerInfoDialog::initDialog(HWND hwnd) {
		this->setCommonControls(hwnd);
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
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			SendMessage(hdlg_ammo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ammo_string.c_str()));
			}
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
			<< my_ttype.getAverageShotEffectRating();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_AVG_SHOT_EFFECT_RATING, my_stream.str().c_str());
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

	TowerPlacedInfoDialog::TowerPlacedInfoDialog(HWND owner, HINSTANCE h_inst, game::Tower& t) :
		InfoDialogBase {h_inst, *t.getBaseType()},
		my_tower {t} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_TOWER_PLACED),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void TowerPlacedInfoDialog::initDialog(HWND hwnd) {
		this->setCommonControls(hwnd);
		// Add ammo types.
		const auto hdlg_ammo = GetDlgItem(hwnd, IDC_INFO_TOWER_AMMO_TYPES);
		std::wstringstream my_stream {};
		for (const auto& st_pair : this->my_tower.getBaseType()->getShotTypes()) {
			my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
				<< (st_pair.second * 100) << L"%";
			const std::wstring ammo_string = st_pair.first->getName() + L": " + my_stream.str();
			my_stream.str(L"");
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			SendMessage(hdlg_ammo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(ammo_string.c_str()));
			}
		}
		if (!this->my_tower.getBaseType()->isWall()) {
			SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_METHOD,
				this->my_tower.getBaseType()->getFiringMethod().getReferenceName().c_str());
			SetDlgItemText(hwnd, IDC_INFO_TOWER_TARGETING_STRATEGY,
				this->my_tower.getBaseType()->getTargetingStrategy().getReferenceName().c_str());
		}
		else {
			SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_METHOD, L"Not Applicable");
			SetDlgItemText(hwnd, IDC_INFO_TOWER_TARGETING_STRATEGY, L"Not Applicable");
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			SendMessage(hdlg_ammo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Not Applicable"));
			}
		}
		SendMessage(hdlg_ammo, CB_SETCURSEL, 0, 0);
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< this->my_tower.getFiringSpeed() << L" / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_SPEED, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< this->my_tower.getFiringRange() << L" cs";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_FIRING_RANGE, my_stream.str().c_str());
		my_stream.str(L"");
		SetDlgItemText(hwnd, IDC_INFO_TOWER_VOLLEY_SHOTS, std::to_wstring(this->my_tower.getVolleyShots()).c_str());
		my_stream << this->my_tower.getReloadDelay() << L" ms";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RELOAD_DELAY, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< this->my_tower.getAverageDamagePerShot();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_EXPECTED_SHOT_DAMAGE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< this->my_tower.getAverageShotEffectRating();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_AVG_SHOT_EFFECT_RATING, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< this->my_tower.getRateOfFire() << L" / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RATE_OF_FIRE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< this->my_tower.getExpectedDPS() << L" dmg / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_EXPECTED_DPS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << L"$" << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< this->my_tower.getCost();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_COST, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(0)
			<< this->my_tower.getRating();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_RATING, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << this->my_tower.getLevel() << L" / " << this->my_tower.getBaseType()->getMaxLevel();
		SetDlgItemText(hwnd, IDC_INFO_TOWER_PLACED_LEVEL, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (this->my_tower.getDamageMultiplier() * 100.0) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_PLACED_DAMAGE_MULTI, my_stream.str().c_str());
		my_stream.str(L"");
		if (this->my_tower.getLevel() >= this->my_tower.getBaseType()->getMaxLevel()) {
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_TOWER_PLACED_UPGRADE_A), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_TOWER_PLACED_UPGRADE_B), FALSE);
		}
		int high_level = 0;
		for (const auto& upgrade : this->my_tower.getBaseType()->getUpgrades()) {
			if (upgrade.getLevel() > high_level) {
				high_level = upgrade.getLevel();
			}
		}
		if (high_level <= this->my_tower.getLevel()) {
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_TOWER_PLACED_UPGRADE_A), FALSE);
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_TOWER_PLACED_UPGRADE_B), FALSE);
		}
	}

	void TowerPlacedInfoDialog::doSell() {
		game::g_my_game->sellTower(static_cast<int>(this->my_tower.getGameX()),
			static_cast<int>(this->my_tower.getGameY()));
	}

	ShotStunInfoDialog::ShotStunInfoDialog(HWND owner, HINSTANCE h_inst, const game::ShotBaseType& stype) :
		InfoDialogBase {h_inst, stype} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_SHOT_STUN),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void ShotStunInfoDialog::initDialog(HWND hwnd) {
		const auto& my_stype = dynamic_cast<const game::StunShotType&>(this->getType());
		std::wstringstream my_stream {};
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getStunChance() * 100.0) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_STUN_CHANCE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << my_stype.getStunDuration() << L" ms";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_STUN_DURATION, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getMultipleStunChance() * 100.0) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_STUN_MULTI_CHANCE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getExtraRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_EXTRA_RATING, my_stream.str().c_str());
		my_stream.str(L"");
	}

	ShotSlowInfoDialog::ShotSlowInfoDialog(HWND owner, HINSTANCE h_inst, const game::ShotBaseType& stype) :
		InfoDialogBase {h_inst, stype} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpet_cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_SHOT_SLOW),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void ShotSlowInfoDialog::initDialog(HWND hwnd) {
		const auto& my_stype = dynamic_cast<const game::SlowShotType&>(this->getType());
		std::wstringstream my_stream {};
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getSlowFactor() * 100.0) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_SLOW_FACTOR, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << my_stype.getSlowDuration() << L" ms";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_SLOW_DURATION, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< (my_stype.getMultipleSlowChance() * 100.0) << L"%";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_SLOW_MULTI_CHANCE, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getExtraRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_EXTRA_RATING, my_stream.str().c_str());
		my_stream.str(L"");
	}

	ShotDoTInfoDialog::ShotDoTInfoDialog(HWND owner, HINSTANCE h_inst, const game::ShotBaseType& stype) :
		InfoDialogBase {h_inst, stype} {
		[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
		DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_SHOT_DOT),
			owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
		}
	}

	void ShotDoTInfoDialog::initDialog(HWND hwnd) {
		const auto& my_stype = dynamic_cast<const game::DoTShotType&>(this->getType());
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TYPE, (*my_stype.getDamageType()).c_str());
		switch (my_stype.getDamageType()) {
		case game::DoTDamageTypes::Poison:
			SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TYPE_DESC, L"Deals poison damage over time"
				L" that has 80% armor piercing and bypasses armor completely.");
			break;
		case game::DoTDamageTypes::Fire:
			SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TYPE_DESC, L"Deals fire damage over time"
				L" that has 50% armor piercing.");
			break;
		case game::DoTDamageTypes::Heal:
			SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TYPE_DESC, L"Restores health over time"
				L" allowing the enemy to survive longer.");
			break;
		default:
			SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TYPE_DESC, L"Error: Description is"
				L" unavaible. Please contact the developer and include the DoT type.");
			break;
		}
		std::wstringstream my_stream {};
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getDamagePerTick();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_DAMAGE_PER_TICK, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << my_stype.getMillisecondsBetweenTicks() << L" ms";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TIME_BETWEEN_TICKS, my_stream.str().c_str());
		my_stream.str(L"");
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TOTAL_TICKS, std::to_wstring(my_stype.getTotalTicks()).c_str());
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_stype.getDoTTotalDuration() << L" s";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_TOTAL_DURATION, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_stype.getDoTDPS() << L" dmg / s";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_SINGLE_DPS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(2)
			<< my_stype.getDoTFullDPS() << L" dmg / s";
		SetDlgItemText(hwnd, IDC_INFO_SHOT_DOT_FULL_DPS, my_stream.str().c_str());
		my_stream.str(L"");
		my_stream << std::setiosflags(std::ios::fixed) << std::setprecision(1)
			<< my_stype.getExtraRating();
		SetDlgItemText(hwnd, IDC_INFO_SHOT_BASE_EXTRA_RATING, my_stream.str().c_str());
		my_stream.str(L"");
	}

	TowerUpgradeInfoDialog::TowerUpgradeInfoDialog(HWND owner, HINSTANCE h_inst, game::Tower& t, game::TowerUpgradeOption upgrade_opt) :
		InfoDialogBase {h_inst, *t.getBaseType()},
		my_tower {t} {
		const auto upgrade_level = this->my_tower.getLevel() + 1;
		for (const auto& upgrade : this->my_tower.getBaseType()->getUpgrades()) {
			if (upgrade_level == upgrade.getLevel() && upgrade_opt == upgrade.getOption()) {
				this->my_upgrade = &upgrade;
				this->upgrade_cost = upgrade.getCostPercent() * this->my_tower.getCost() + 1.0;
				break;
			}
		}
		if (!this->my_upgrade) {
			MessageBox(owner, L"Error: Could not find tower upgrade information.", L"Upgrade Dialog Creation Failed", MB_OK | MB_ICONWARNING);
		}
		else {
			[[gsl::suppress(26490)]] { // C26490 => Do not use reinterpret_cast.
			DialogBoxParam(this->getApplicationHandle(), MAKEINTRESOURCE(IDD_INFO_TOWER_UPGRADE),
				owner, InfoDialogBase::infoDialogProc, reinterpret_cast<LPARAM>(this));
			}
		}
	}

	void TowerUpgradeInfoDialog::initDialog(HWND hwnd) {
		std::wstringstream my_stream {};
		SetDlgItemText(hwnd, IDC_INFO_BASE_NAME, this->my_tower.getBaseType()->getName().c_str());
		switch (this->my_upgrade->getSpecial()) {
		case game::TowerUpgradeSpecials::None:
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, L"This upgrade does not provide the tower"
				L" with any new special abilities.");
			break;
		case game::TowerUpgradeSpecials::Extra_Cash:
			my_stream << L"At the end of each level, this tower has a " << std::setiosflags(std::ios_base::fixed)
				<< std::setprecision(0) << (this->my_upgrade->getSpecialChance() * 100.0) << L"% chance to add an extra $"
				<< std::setiosflags(std::ios_base::fixed) << std::setprecision(2) << this->my_upgrade->getSpecialPower()
				<< L" to the money you receive at the end of the level.";
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, my_stream.str().c_str());
			break;
		case game::TowerUpgradeSpecials::Multishot:
			my_stream << L"Each time this tower fires a shot, it has a " << std::setiosflags(std::ios_base::fixed)
				<< std::setprecision(0) << (this->my_upgrade->getSpecialChance() * 100.0) << L"% chance to generate "
				<< std::setiosflags(std::ios_base::fixed) << std::setprecision(0) << this->my_upgrade->getSpecialPower()
				<< L" additional shot(s) at no additional cost.";
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, my_stream.str().c_str());
			break;
		case game::TowerUpgradeSpecials::Mega_Missile:
			my_stream << L"Each time this tower fires a shot, it has a " << std::setiosflags(std::ios_base::fixed)
				<< std::setprecision(0) << (this->my_upgrade->getSpecialChance() * 100.0) << L"% chance to replace"
				<< L" the shot with a powerful 'Mega Missile'.";
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, my_stream.str().c_str());
			break;
		case game::TowerUpgradeSpecials::Fast_Reload:
			my_stream << L"Each time this tower has to reload, there is a " << std::setiosflags(std::ios_base::fixed)
				<< std::setprecision(0) << (this->my_upgrade->getSpecialChance() * 100.0) << L"% chance to have its"
				L" reload time reduced by " << std::setiosflags(std::ios_base::fixed) << std::setprecision(0)
				<< (this->my_upgrade->getSpecialPower() * 100.0) << L"%.";
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, my_stream.str().c_str());
			break;
		default:
			SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_SPECIAL_DESC, L"Description unavailable. Please report to developer.");
			break;
		}
		my_stream.str(L"");
		SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_LEVEL, std::to_wstring(this->my_upgrade->getLevel()).c_str());
		my_stream << L"$" << std::setiosflags(std::ios_base::fixed) << std::setprecision(2) << this->upgrade_cost;
		SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_COST, my_stream.str().c_str());
		my_stream.str(L"");
		static const COLORREF red_color {RGB(128, 0, 0)};
		static const COLORREF blue_color {RGB(0, 0, 128)};
		static const COLORREF green_color {RGB(0, 128, 0)};
		const auto my_change_field_lambda = [this, &my_stream, &hwnd](int resource_id, double new_value) noexcept {
			my_stream << std::setiosflags(std::ios_base::fixed) << std::setprecision(1)
				<< (new_value * 100.0) << L"%";
			SetDlgItemText(hwnd, resource_id, my_stream.str().c_str());
			my_stream.str(L"");
		};
		const auto my_color_choose_lambda = [this, &hwnd](int resource_id, double old_value, double new_value, bool invert_colors = false) noexcept {
			if (new_value > old_value) {
				this->addControlColor(resource_id, invert_colors ? red_color : green_color);
			}
			else if (new_value < old_value) {
				this->addControlColor(resource_id, invert_colors ? green_color : red_color);
			}
			else {
				this->addControlColor(resource_id, blue_color);
			}
			RedrawWindow(GetDlgItem(hwnd, resource_id), nullptr, nullptr, RDW_INVALIDATE);
		};
		const double old_speed_multi = this->my_tower.fire_speed_multiplier;
		const double old_range_multi = this->my_tower.fire_range_multiplier;
		const double old_ammo_multi = this->my_tower.volley_shots_multiplier;
		const double old_delay_multi = this->my_tower.reload_delay_multiplier;
		const double old_dmg_multi = this->my_tower.dmg_multiplier;
		const double new_speed_multi = old_speed_multi * this->my_upgrade->getSpeedMultiplier();
		const double new_range_multi = old_range_multi * this->my_upgrade->getRangeMultiplier();
		const double new_ammo_multi = old_ammo_multi * this->my_upgrade->getAmmoMultiplier();
		const double new_delay_multi = old_delay_multi * this->my_upgrade->getDelayMultiplier();
		const double new_dmg_multi = old_dmg_multi * this->my_upgrade->getDamageMultiplier();
		my_change_field_lambda(IDC_INFO_TOWER_UPGRADE_SPEED, new_speed_multi);
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_SPEED, old_speed_multi, new_speed_multi);
		my_change_field_lambda(IDC_INFO_TOWER_UPGRADE_RANGE, new_range_multi);
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_RANGE, old_range_multi, new_range_multi);
		my_change_field_lambda(IDC_INFO_TOWER_UPGRADE_AMMO, new_ammo_multi);
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_AMMO, old_ammo_multi, new_ammo_multi);
		my_change_field_lambda(IDC_INFO_TOWER_UPGRADE_DELAY, new_delay_multi);
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_DELAY, old_delay_multi, new_delay_multi, true);
		my_change_field_lambda(IDC_INFO_TOWER_UPGRADE_DAMAGE, new_dmg_multi);
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_DAMAGE, old_dmg_multi, new_dmg_multi);
		const auto old_dps = this->my_tower.getExpectedDPS();
		const auto new_dps = game::towers::getExpectedDPS(game::towers::getAverageDamagePerShot(this->my_tower.getBaseType()->getShotTypes(),
			new_dmg_multi), game::towers::getRateOfFire(this->my_tower.getBaseType()->getFiringSpeed() * new_speed_multi,
				static_cast<int>(this->my_tower.getBaseType()->getVolleyShots() * new_ammo_multi),
				static_cast<int>(this->my_tower.getBaseType()->getReloadDelay() * new_delay_multi),
				this->my_tower.getBaseType()->isWall()));
		my_stream << std::setiosflags(std::ios_base::fixed) << std::setprecision(2)
			<< new_dps << L" dmg / s";
		SetDlgItemText(hwnd, IDC_INFO_TOWER_UPGRADE_EXPECTED_DPS, my_stream.str().c_str());
		my_stream.str(L"");
		my_color_choose_lambda(IDC_INFO_TOWER_UPGRADE_EXPECTED_DPS, old_dps, new_dps);
		// Not enough money to buy, so don't give the option.
		if (game::g_my_game->getPlayerCash() < this->upgrade_cost) {
			EnableWindow(GetDlgItem(hwnd, IDC_INFO_TOWER_UPGRADE_DO_UPGRADE), FALSE);
		}
	}

	void TowerUpgradeInfoDialog::doUpgrade() {
		game::g_my_game->changePlayerCash(-this->upgrade_cost);
		this->my_tower.upgradeTower(this->my_upgrade->getLevel(), this->my_upgrade->getOption());
	}
}

