#pragma once
#pragma comment(lib, "xaudio2.lib")
#include "./../targetver.hpp"
#include <xaudio2.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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

		// It probably shouldn't be a template as it was specifically made
		// for deleting XAUDIO2_BUFFERs.
		template <typename T>
		struct DeleteBuffer {
			void operator()(T* pT) {
				delete[] pT->pAudioData;
				pT->pAudioData = nullptr;
				delete pT;
			}
		};

		/// <summary>This class stores and manages all audio resources.</summary>
		class AudioResources {
		public:
			AudioResources() :
				xaudio2 {nullptr},
				master_voice {nullptr},
				song_voices {},
				song_buffers {},
				current_song {-1},
				music_volume {0.1f},
				mute_music {false} {
				// Probably throw an exception for failure...
				if (!this->initAudio()) {
					throw std::runtime_error {"Initialization of audio failed."};
				}
			}
			// Make sure to stop playing any active music.
			~AudioResources() {
				for (auto& voice : this->song_voices) {
					voice->Stop();
					voice->FlushSourceBuffers();
				}
			}

			/// <summary>Loads a song from the given file into a buffer. This method throws
			/// an exception if the loading fails.</summary>
			/// <param name="file_name">The name of the file to load.</param>
			void loadSong(std::wstring file_name);
			/// <summary>Plays music from the preloaded songs. This method throws an
			/// exception if the playing fails.</summary>
			/// <param name="index">The index of the song based on the order the songs were loaded.</param>
			void playSong(int index);
			/// <summary>Sets the volume of music.</summary>
			/// <param name="new_volume">The new volume of music to use. A 1.0 means
			/// that the original audio file is played at its normal volume. A 0.0 means
			/// that the audio is muted.</param>
			void setVolume(float new_volume);
			/// <summary>Starts the playback of music.</summary>
			void startMusic();
			/// <summary>Stops the playback of music.</summary>
			void stopMusic();
			// Getters
			float getMusicVolume() const noexcept {
				return this->music_volume;
			}
			bool isMusicMuted() const noexcept {
				return this->mute_music;
			}
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
			/// <summary>Pointer to the XAudio2 source voice used to play songs.</summary>
			std::vector<std::unique_ptr<IXAudio2SourceVoice, DestroyVoice<IXAudio2SourceVoice>>> song_voices;
			/// <summary>Stores the buffers containing the information of loaded songs.</summary>
			std::vector<std::unique_ptr<XAUDIO2_BUFFER, DeleteBuffer<XAUDIO2_BUFFER>>> song_buffers;
			/// <summary>Stores the current song being played (so it can be stopped later.)</summary>
			int current_song;
			float music_volume;
			bool mute_music;
		};


		// Global variables
		// It would be a pain to have to pass this as a parameter everywhere I go.
		/// <summary>Global pointer to the audio player.</summary>
		extern std::unique_ptr<AudioResources> g_my_audio;
		// Global constants; I should update these whenever I change the music selection.
		// Actually, these ought to be an enumeration but this is slightly more convenient.
		constexpr const auto town_index = 0;
		constexpr const auto boss_index = 1;
		constexpr const auto level_index = 2;
		constexpr const auto gameover_index = 3;
		constexpr const auto victory_index = 4;
	}
}
