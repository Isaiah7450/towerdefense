#pragma once
// File Author: Isaiah Hoffman
// File Created: March 31, 2018
// A lot of code adapted from the Windows 7 Common Item Dialog sample.
#include "./../targetver.hpp"
#include <windows.h>
#include <objbase.h>
#include <propvarutil.h>
#include <propkey.h>
#include <propidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <shtypes.h>
#include <memory>
#include "graphics_DX.hpp"

namespace hoffman_isaiah::winapi {
	// This is a stupidly high amount of extra code to implement just to be able to
	// create file dialogs...
	class MyFileDialogHandler : public IFileDialogEvents, public IFileDialogControlEvents {
	public:
		// Most of this code is more or less copied from the github src of the
		// sample for the common item dialog box.
		IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
			static const QITAB qit[] = {
				QITABENT(MyFileDialogHandler, IFileDialogEvents),
				QITABENT(MyFileDialogHandler, IFileDialogControlEvents)
			};
			return QISearch(this, qit, riid, ppv);
		}

		/// <summary>Adds a reference to this interface.</summary>
		/// <returns>The new reference count.</returns>
		IFACEMETHODIMP_(ULONG) AddRef() override {
			return InterlockedIncrement(&this->cref);
		}
		/// <summary>Removes a reference to this interface.</summary>
		/// <returns>The new reference count.</returns>
		IFACEMETHODIMP_(ULONG) Release() override {
			const long my_cref = InterlockedIncrement(&this->cref);
			if (!my_cref) {
				delete this;
			}
			return my_cref;
		}

		// Event handler implementations.
		// The fact that all of these can't have default implementations to begin with is annoying.
		// IFileDialogEvents methods.
		IFACEMETHODIMP OnFileOk(IFileDialog*) override {
			return S_OK;
		}
		IFACEMETHODIMP OnFolderChange(IFileDialog*) override {
			return S_OK;
		}
		IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) override {
			return S_OK;
		}
		/*
		IFACEMETHODIMP OnHelp(IFileDialog*) override {
			return S_OK;
		}
		*/
		IFACEMETHODIMP OnSelectionChange(IFileDialog*) override {
			return S_OK;
		}
		IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) override {
			return S_OK;
		}
		/// <param name="pfd">Pointer to the file dialog.</param>
		IFACEMETHODIMP OnTypeChange(IFileDialog *pfd) override {
			UNREFERENCED_PARAMETER(pfd);
			return S_OK;
		}
		IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) override {
			return S_OK;
		}

		// IFileDialogControlEvents methods.
		IFACEMETHODIMP OnItemSelected(IFileDialogCustomize*, DWORD, DWORD) override {
			return S_OK;
		}
		IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD) override {
			return S_OK;
		}
		IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL) override {
			return S_OK;
		}
		IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD) override {
			return S_OK;
		}
	protected:
		virtual ~MyFileDialogHandler() noexcept = default;
	private:
		/// <summary>The current reference count for the interface.</summary>
		long cref;
	};
}

