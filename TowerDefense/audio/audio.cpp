// Created by: Isaiah Hoffman
// Created on: March 10, 2022
#include "audio.hpp"
#include "./../targetver.hpp"
#include <Windows.h>
#include <xaudio2.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace hoffman_isaiah::audio {
// Following preprocessor code adapted from official docs.
#ifdef _XBOX // Big Endian
	constexpr const auto fourccRIFF = 'RIFF';
	constexpr const auto fourccDATA = 'data';
	constexpr const auto fourccFMT = 'fmt ';
	constexpr const auto fourccWAVE = 'WAVE';
	constexpr const auto fourccXWMA = 'XWMA';
	constexpr const auto fourccDPDS = 'dpds';
#else // Little Endian
	constexpr const auto fourccRIFF = 'FFIR';
	constexpr const auto fourccDATA = 'atad';
	constexpr const auto fourccFMT = ' tmf';
	constexpr const auto fourccWAVE = 'EVAW';
	constexpr const auto fourccXWMA = 'AMWX';
	constexpr const auto fourccDPDS = 'sdpd';
#endif
	bool AudioResources::initAudio() {
		IXAudio2* raw_xaudio2 = nullptr;
		HRESULT hr;
		if (FAILED(hr = XAudio2Create(&raw_xaudio2, 0U, XAUDIO2_DEFAULT_PROCESSOR))) {
			return false;
		}
		this->xaudio2.reset(raw_xaudio2);
		IXAudio2MasteringVoice* raw_master_voice = nullptr;
		if (FAILED(hr = this->xaudio2->CreateMasteringVoice(&raw_master_voice))) {
			return false;
		}
		this->master_voice.reset(raw_master_voice);
		this->master_voice->SetVolume(0.1f);
		return true;
	}

	void AudioResources::loadSong(std::wstring file_name) {
		// Open file.
		HANDLE my_file = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		try {
			if (INVALID_HANDLE_VALUE == my_file
				|| INVALID_SET_FILE_POINTER == SetFilePointer(my_file, 0, nullptr, FILE_BEGIN)) {
				throw std::runtime_error {"Could not open file."};
			}
			// Code adapted from XAudio2 documentation (How to Load Audio Data Files in XAudio2)
			// Locate RIFF chunk and check file type.
			DWORD dw_chunk_size;
			DWORD dw_chunk_position;
			this->findChunk(my_file, audio::fourccRIFF, dw_chunk_size, dw_chunk_position);
			DWORD file_type;
			this->readChunk(my_file, &file_type, sizeof(DWORD), dw_chunk_position);
			if (file_type != fourccWAVE) {
				throw std::runtime_error {"Expected a WAV file."};
			}
			// Locate FORMAT chunk and and copy into structure.
			WAVEFORMATEXTENSIBLE wfx {0};
			wfx.Format.nSamplesPerSec = 44100;
			wfx.Format.nChannels = 1;
			wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
			this->findChunk(my_file, audio::fourccFMT, dw_chunk_size, dw_chunk_position);
			this->readChunk(my_file, &wfx, dw_chunk_size, dw_chunk_position);
			// Locate DATA chunk and read into buffer.
			this->findChunk(my_file, audio::fourccDATA, dw_chunk_size, dw_chunk_position);
			auto data_buffer = std::make_unique<BYTE[]>(dw_chunk_size);
			this->readChunk(my_file, data_buffer.get(), dw_chunk_size, dw_chunk_position);
			// Populate XAUDIO2_BUFFER
			// (I couldn't get this to work directly with std::make_unique<>... It's probably
			// simple; I just need to look it up later.)
			std::unique_ptr<XAUDIO2_BUFFER, DeleteBuffer<XAUDIO2_BUFFER>> buffer {nullptr};
			auto raw_buffer = new XAUDIO2_BUFFER;
			buffer.reset(raw_buffer);
			buffer->LoopBegin = 0;
			buffer->LoopCount = XAUDIO2_LOOP_INFINITE;
			buffer->LoopLength = 0;
			buffer->PlayBegin = 0;
			buffer->PlayLength = 0;
			buffer->pContext = nullptr;
			buffer->AudioBytes = dw_chunk_size;
			// The custom deleter will make sure this is deleted properly.
			buffer->pAudioData = data_buffer.release();
			buffer->Flags = XAUDIO2_END_OF_STREAM;
			// Rest is adapted from XAudio2 documentation (How to: Play a Sound with XAudio2)
			// Create source voice.
			IXAudio2SourceVoice* raw_source_voice;
			HRESULT hr;
			if (FAILED(hr = this->xaudio2->CreateSourceVoice(&raw_source_voice,
				&wfx.Format))) {
				throw std::runtime_error {"Could not create source voice."};
			}
			std::unique_ptr<IXAudio2SourceVoice, DestroyVoice<IXAudio2SourceVoice>> source_voice
			{raw_source_voice};
			// Submit audio buffer.
			if (FAILED(hr = source_voice->SubmitSourceBuffer(buffer.get()))) {
				throw std::runtime_error {"Could not provide buffer to source voice."};
			}
			// Transfer ownership.
			this->song_buffers.push_back(std::move(buffer));
			this->song_voices.push_back(std::move(source_voice));

		}
		catch (const std::runtime_error&) {
			if (INVALID_HANDLE_VALUE == my_file || !my_file) {
				CloseHandle(my_file);
			}
			throw;
		}
		CloseHandle(my_file);
	}

	void AudioResources::playSong(int index) {
		if (this->current_song >= 0 && index != this->current_song) {
			if (FAILED(this->song_voices.at(this->current_song)->Stop(0U))) {
				throw std::runtime_error {"Failed to stop playing current song."};
			}
		}
		this->current_song = index;
		this->song_voices.at(index)->Start(0U);
	}

	void AudioResources::findChunk(HANDLE my_file, DWORD fourcc, DWORD& chunk_size,
		DWORD& chunk_data_position) const {
		// Adapted from documentation for XAudio2 (How to Load Audio Data Files in XAudio2)
		if (INVALID_SET_FILE_POINTER == SetFilePointer(my_file, 0, nullptr, FILE_BEGIN)) {
			throw std::runtime_error {"Could not reset file pointer."};
		}
		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;
		HRESULT hr = S_OK;
		while (hr == S_OK) {
			DWORD dwRead;
			if (FALSE == ReadFile(my_file, &dwChunkType, sizeof(DWORD), &dwRead, NULL)) {
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			if (FALSE == ReadFile(my_file, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL)) {
				hr = HRESULT_FROM_WIN32(GetLastError());
			}

			switch (dwChunkType) {
			case audio::fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (FALSE == ReadFile(my_file, &dwFileType, sizeof(DWORD), &dwRead, NULL)) {
					hr = HRESULT_FROM_WIN32(GetLastError());
				}
				break;
			default:
				if (INVALID_SET_FILE_POINTER
					== SetFilePointer(my_file, dwChunkDataSize, NULL, FILE_CURRENT)) {
					throw std::runtime_error {"Could not reset file pointer."};
				}
				break;
			}
			dwOffset += sizeof(DWORD) * 2;
			if (dwChunkType == fourcc) {
				chunk_size = dwChunkDataSize;
				chunk_data_position = dwOffset;
				return;
			}
			dwOffset += dwChunkDataSize;
			if (bytesRead >= dwRIFFDataSize) {
				throw std::runtime_error {"Reached end of file before expected."};
			}
		}
	}

	void AudioResources::readChunk(HANDLE my_file, void* buffer, DWORD buffer_size,
		DWORD buffer_offset) const {
		// Adapted from XAudio2 documentation (How to Load Audio Data Files in XAudio2)
		if (INVALID_SET_FILE_POINTER == SetFilePointer(my_file, buffer_offset, nullptr, FILE_BEGIN)) {
			throw std::runtime_error {"Could not change position of file pointer."};
		}
		DWORD dw_read;
		if (FALSE == ReadFile(my_file, buffer, buffer_size, &dw_read, nullptr)) {
			throw std::runtime_error {"File read operation failed."};
		}
	}
}
