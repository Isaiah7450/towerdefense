#pragma once
#pragma comment(lib, "xaudio2.lib")
#include "./../targetver.hpp"
#include <xaudio2.h>
#include <memory>
#include <string>

// Created by: Isaiah Hoffman
// Created on: March 10, 2022
namespace hoffman_isaiah {
	namespace audio {
		// For use with COM pointers
		template <typename T>
		struct ReleaseCOM {
			void operator()(T* pT) {
				pT->Release();
			}
		};

		// For some arcane reason, the cleanup method is different for voices.
		template <typename T>
		struct DestroyVoice {
			void operator()(T* pT) {
				pT->DestroyVoice();
			}
		};

		/// <summary>This class stores and manages all audio resources.</summary>
		class AudioResources {
		public:
			AudioResources() :
				xaudio2 {nullptr},
				master_voice {nullptr} {
				// Probably throw an exception for failure...
				this->initAudio();
			}

			/// <summary>Plays music from the given file.</summary>
			/// <param name="file_name">The name of the file to load.</param>
			void playMusic(std::wstring file_name);
		private:
			/// <summary>Initializes the audio engine.</summary>
			/// <returns>True if the action succeeded otherwise false.</returns>
			bool initAudio();

			/// <summary>Finds a chunk in a .WAV file and stores the needed information.
			/// This method throws an exception if an error occurs.</summary>
			/// <param name="my_file">Handle to the file to read.</param>
			/// <param name="fourcc">The code to search for, such as "riff", "fmt", or "data".</param>
			/// <param name="chunk_size">Reference to a variable to store the chunk's size.</param>
			/// <param name="chunk_data_position">Reference to a variable to store the location
			/// of the chunk.</param>
			void findChunk(HANDLE my_file, DWORD fourcc, DWORD& chunk_size, DWORD& chunk_data_position) const;
			/// <summary>Reads the data associated with a data chunk's offset and size into
			/// a buffer. An exception is thrown if an error occurs.</summary>
			/// <param name="my_file">Handle to the file to read.</param>
			/// <param name="buffer">The buffer to read into.</param>
			/// <param name="buffer_size">The size of the buffer.</param>
			/// <param name="buffer_offset">The offset to start reading from.</param>
			void readChunk(HANDLE my_file, void* buffer, DWORD buffer_size, DWORD buffer_offset) const;

			// Order dependency note: this should be released LAST.
			/// <summary>Pointer to the XAudio2 engine.</summary>
			std::unique_ptr<IXAudio2, ReleaseCOM<IXAudio2>> xaudio2;
			/// <summary>Pointer to the XAudio2 master voice.</summary>
			std::unique_ptr<IXAudio2MasteringVoice, DestroyVoice<IXAudio2MasteringVoice>> master_voice;
		};
	}
}
