#pragma once
// Author: Isaiah Hoffman
// Created: March 5, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>

namespace hoffman::isaiah::game {
	// Forward declarations.
	class EnemyType;
	class ShotBaseType;
	class TowerType;
	class GameObjectType;
}

namespace hoffman::isaiah::winapi {
	/// <summary>The base class for info dialog boxes.</summary>
	class InfoDialogBase {
	public:
		// Dialog box procedure.
		static INT_PTR CALLBACK infoDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	protected:
		/// <summary>Creates a new info dialog box.</summary>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		/// <param name="object_type">The object type to show info for.</param>
		InfoDialogBase(HINSTANCE h_inst, const game::GameObjectType& object_type);
		/// <summary>Initializes the dialog.</summary>
		/// <param name="hwnd">Handle to the dialog box window.</param>
		virtual void initDialog(HWND hwnd) = 0;

		// Getters
		HINSTANCE getApplicationHandle() const noexcept {
			return this->h_instance;
		}
		const game::GameObjectType& getType() const noexcept {
			return this->my_type;
		}
	private:
		/// <summary>The handle to the application instance.</summary>
		HINSTANCE h_instance;
		/// <summary>The object type to display info for.</summary>
		const game::GameObjectType& my_type;
	};
	
	/// <summary>Displays information about an enemy.</summary>
	class EnemyInfoDialog : public InfoDialogBase {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		/// <param name="etype">The enemy type to display information for.</param>
		EnemyInfoDialog(HWND owner, HINSTANCE h_inst, const game::EnemyType& etype);
	protected:
		// Implements InfoDialogBase::initDialog().
		void initDialog(HWND hwnd) override;
	private:
		std::wstring hp_string {};
		std::wstring ahp_string {};
		std::wstring ar_string {};
		std::wstring pt_string {};
		std::wstring walk_string {};
		std::wstring run_string {};
		std::wstring injured_string {};
		std::wstring fly_string {};
		std::wstring dmg_string {};
		std::wstring num_buffs_string {};
		std::wstring strat_string {};
		std::wstring diag_string {};
		std::wstring buff_rating_string {};
		std::wstring rating_string {};
	};
}
