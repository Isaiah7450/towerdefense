#pragma once
// File Author: Isaiah Hoffman
// File Created: March 31, 2018
#include "./../targetver.hpp"
#include <Windows.h>
#include <Shobjidl.h>

namespace hoffman::isaiah {
	namespace graphics::dialogs {
		class TerrainEditorOpenDialog : public IFileDialogEvents {
		public:
			HRESULT OnFileOk(__in IFileDialog* my_dialog) override {
				return S_OK;
			}
			HRESULT OnFolderChange(__in IFileDialog* my_dialog) override {
				return S_OK;
			}
		};
	}
}

#pragma comment(lib: "Shobjidl.idl")