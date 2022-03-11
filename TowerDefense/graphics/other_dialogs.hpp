#pragma once
// Author: Isaiah Hoffman
// Created: March 11, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include "./../globals.hpp"

namespace hoffman_isaiah::game {
	// Forward declarations
	class MyGame;
}

namespace hoffman_isaiah::winapi {
	/// <summary>Represents a dialog used to select the challenge level of the game.</summary>
	class ChallengeLevelDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		ChallengeLevelDialog(HWND owner, HINSTANCE h_inst);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		// Getters
		int getChallengeLevel() const noexcept {
			return this->selected_clevel;
		}
	protected:
		// Implements IDialog::initDialog(HWND)
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>The challenge level selected by the user.</summary>
		int selected_clevel {1};
	};

	/// <summary>Represents a dialog box that is used to start games on custom maps.</summary>
	class StartCustomGameDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		StartCustomGameDialog(HWND owner, HINSTANCE h_inst);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		// Getters
		int getChallengeLevel() const noexcept {
			return this->selected_clevel;
		}
		std::wstring getMapName() const noexcept {
			return this->map_name;
		}
	protected:
		// Implements IDialog::initDialog(HWND)
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>The challenge level selected by the user.</summary>
		int selected_clevel {1};
		/// <summary>The name of the map to load.</summary>
		std::wstring map_name {L"default"};
	};

	/// <summary>This dialog box stores the current map and music settings.</summary>
	class SettingsDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the owning window.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		SettingsDialog(HWND owner, HINSTANCE h_inst);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	protected:
		void initDialog(HWND hwnd) override;
	private:
	};

	/// <summary>Represents a dialog that shows statistics for the player common across all games.</summary>
	class GlobalStatsDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		/// <param name="my_game">Reference to the game state.</param>
		GlobalStatsDialog(HWND owner, HINSTANCE h_inst, const game::MyGame& my_game);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		// Getters
		long long getHiscore() const noexcept {
			return this->highest_score;
		}
		const std::map<int, int>& getHighestLevels() const noexcept {
			return this->highest_levels;
		}
	protected:
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>The player's highest score ever.</summary>
		long long highest_score;
		/// <summary>The highest levels the player has reached on a particular difficulty.</summary>
		std::map<int, int> highest_levels;
	};

	/// <summary>This dialog box provides general information about the program and its maker.</summary>
	class HelpAboutDialog : public IDialog {
	public:
		HelpAboutDialog(HWND owner, HINSTANCE h_inst);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	protected:
		void initDialog(HWND hwnd);
	};

	/// <summary>Represents a dialog used to make a new map in the terrain editor.</summary>
	class TerrainEditorNewMapDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		/// <param name="default_name">The default new map name.</param>
		TerrainEditorNewMapDialog(HWND owner, HINSTANCE h_inst, std::wstring default_name);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		// Getters
		bool isGood() const noexcept {
			return this->create_new_map;
		}
		std::wstring getName() const noexcept {
			return this->map_name;
		}
		int getRows() const noexcept {
			return this->num_rows;
		}
		int getColumns() const noexcept {
			return this->num_cols;
		}
	protected:
		// Implements IDialog::initDialog(HWND)
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>Handle to the dialog.</summary>
		HWND hdlg {nullptr};
		/// <summary>Handle to the rows (up-down) control.</summary>
		HWND hwnd_rows_select {nullptr};
		/// <summary>Handle to the columns (up-down) control.</summary>
		HWND hwnd_cols_select {nullptr};
		/// <summary>Proceed with creation?</summary>
		bool create_new_map {false};
		/// <summary>The new map name.</summary>
		std::wstring map_name;
		/// <summary>The number of rows in the new map.</summary>
		int num_rows {40};
		/// <summary>The number of columns in the new map.</summary>
		int num_cols {35};
	};

	/// <summary>Represents a dialog used to make a new map in the terrain editor.</summary>
	class TerrainEditorOpenMapDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		TerrainEditorOpenMapDialog(HWND owner, HINSTANCE h_inst);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		// Getters
		bool isGood() const noexcept {
			return this->open_map;
		}
		std::wstring getName() const noexcept {
			return this->map_name;
		}
	protected:
		// Implements IDialog::initDialog(HWND)
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>Go forward with opening the map?</summary>
		bool open_map {false};
		/// <summary>The name of the map to load.</summary>
		std::wstring map_name {L"default"};
	};

	/// <summary>Represents a dialog used to save the current map under a different name in the terrain editor.</summary>
	class TerrainEditorSaveMapAsDialog : public IDialog {
	public:
		/// <param name="owner">Handle to the window that owns this dialog box.</param>
		/// <param name="h_inst">The hInstance parameter given by the WinMain function.</param>
		/// <param name="default_name">The default save name.</param>
		TerrainEditorSaveMapAsDialog(HWND owner, HINSTANCE h_inst, std::wstring default_name);
		// Dialog box procedure.
		static INT_PTR CALLBACK dialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		// Getters
		bool isGood() const noexcept {
			return this->save_map;
		}
		bool showOvewriteConfirmation() const noexcept {
			return this->show_overwrite_confirm;
		}
		std::wstring getName() const noexcept {
			return this->map_name;
		}
	protected:
		// Implements IDialog::initDialog(HWND)
		void initDialog(HWND hwnd) override;
	private:
		/// <summary>Proceed with saving?</summary>
		bool save_map {false};
		/// <summary>Show overwrite confirmation?</summary>
		bool show_overwrite_confirm {true};
		/// <summary>The new save name.</summary>
		std::wstring map_name;
	};
}
