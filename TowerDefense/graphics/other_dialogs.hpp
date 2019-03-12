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
}
