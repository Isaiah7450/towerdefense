#pragma once
// Author: Isaiah Hoffman
// Created: March 11, 2019
#include "./../targetver.hpp"
#include <Windows.h>
#include <memory>
#include <string>
#include "./../globals.hpp"

namespace hoffman::isaiah::winapi {
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
		int selected_clevel {1};
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
}
