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

namespace hoffman::isaiah::winapi {
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
		IFACEMETHODIMP_(ULONG) AddRef() {
			return InterlockedIncrement(&this->cref);
		}
		/// <summary>Removes a reference to this interface.</summary>
		/// <returns>The new reference count.</returns>
		IFACEMETHODIMP_(ULONG) Release() {
			long my_cref = InterlockedIncrement(&this->cref);
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

	class TerrainEditorOpenDialog : public MyFileDialogHandler {
	public:

		// Mostly copied from online help files, modified as needed.
		/// <summary>Utility function to create an instance of this event handler.</sumamry>
		///
		static HRESULT createInstance(REFIID riid, void **ppv) {
			*ppv = nullptr;
			// ComPtr hence the usage of new.
			auto dialog_event_handler = new TerrainEditorOpenDialog();
			HRESULT hr = dialog_event_handler ? S_OK : E_OUTOFMEMORY;
			if (SUCCEEDED(hr)) {
				hr = dialog_event_handler->QueryInterface(riid, ppv);
				dialog_event_handler->Release();
			}
			return hr;
		}
	};
}

// #pragma comment(lib, "Shobjidl.idl")