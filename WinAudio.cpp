#include "WinAudio.h"
#include <Mmreg.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <cstring>

WinAudio* WinAudio::WIN_AUDIO = NULL;

// Constants for reverb
const int REVERB_DELAY_MS = 100; // Delay in milliseconds
const float REVERB_DECAY = 0.6f; // Decay factor

WinAudio::WinAudio(HWND hwnd, int sample_rate) {
    // Initialize variables
    m_CurWaveOut = 0;
    m_SampleRate = sample_rate;
    m_IsReleasing = false;

    // Specify output parameters
    m_Format.wFormatTag = WAVE_FORMAT_PCM;        // simple, uncompressed format
    m_Format.nChannels = 2;                       // 1=mono, 2=stereo
    m_Format.nSamplesPerSec = sample_rate;        // sample rate
    m_Format.nAvgBytesPerSec = sample_rate * 2 * 2; // nSamplesPerSec * n.Channels * wBitsPerSample/8
    m_Format.nBlockAlign = 4;                     // n.Channels * wBitsPerSample/8
    m_Format.wBitsPerSample = 16;                 // 16 for high quality, 8 for telephone-grade
    m_Format.cbSize = 0;                          // must be set to zero

    // Set handle
    assert(WIN_AUDIO == NULL);
    WIN_AUDIO = this;

    // Initialize reverb buffer
    m_ReverbBufferSize = (REVERB_DELAY_MS * sample_rate / 1000) * 2; // Stereo samples
    m_ReverbBuffer.resize(m_ReverbBufferSize, 0);
    m_ReverbBufferIndex = 0;
}

WinAudio::~WinAudio() {
    stop();
}

bool WinAudio::play() {
    MMRESULT result;
    CHAR fault[256];

    // Create a mutex
    m_Mutex = CreateMutex(NULL, FALSE, NULL);
    if (m_Mutex == NULL) {
        std::cout << "Mutex failed.";
        return false;
    }

    // Open the audio driver
    m_IsReleasing = false;
    result = waveOutOpen(&m_HWaveOut, WAVE_MAPPER, &m_Format, (DWORD_PTR)Callback, NULL, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR) {
        waveInGetErrorText(result, fault, 256);
        std::cout << fault << std::endl;
        return false;
    }

    // Set up and prepare header for output
    for (int i = 0; i < NUM_AUDIO_BUFFS; i++) {
        memset(m_WaveOut[i], 0, AUDIO_BUFF_SIZE * sizeof(int16_t));
        memset(&m_WaveOutHdr[i], 0, sizeof(WAVEHDR));
        m_WaveOutHdr[i].lpData = (LPSTR)m_WaveOut[i];
        m_WaveOutHdr[i].dwBufferLength = AUDIO_BUFF_SIZE * sizeof(int16_t);
        result = waveOutPrepareHeader(m_HWaveOut, &m_WaveOutHdr[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            waveInGetErrorText(result, fault, 256);
            std::cout << fault << std::endl;
        }
    }

    SubmitBuffer();
    SubmitBuffer();
    return true;
}

bool WinAudio::stop() {
    MMRESULT result;
    CHAR fault[256];
    if (m_IsReleasing) {
        return true;
    }
    WaitForSingleObject(m_Mutex, INFINITE);
    m_IsReleasing = true;
    for (int i = 0; i < NUM_AUDIO_BUFFS; i++) {
        result = waveOutReset(m_HWaveOut);
        if (result != MMSYSERR_NOERROR) {
            waveInGetErrorText(result, fault, 256);
            std::cout << fault << std::endl;
            return false;
        }
        result = waveOutUnprepareHeader(m_HWaveOut, &m_WaveOutHdr[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            waveInGetErrorText(result, fault, 256);
            std::cout << fault << std::endl;
            return false;
        }
    }
    waveOutClose(m_HWaveOut);
    ReleaseMutex(m_Mutex);
    return true;
}

void WinAudio::SubmitBuffer() {
    MMRESULT result;
    CHAR fault[256];

    // Reject if releasing
    if (m_IsReleasing) { return; }

    // Write the audio to the sound card
    WaitForSingleObject(m_Mutex, INFINITE);
    result = waveOutWrite(m_HWaveOut, &m_WaveOutHdr[m_CurWaveOut], sizeof(WAVEHDR));

    // Check for errors
    if (result != MMSYSERR_NOERROR) {
        waveInGetErrorText(result, fault, 256);
        std::cout << fault << std::endl;
    }
    m_CurWaveOut = (m_CurWaveOut + 1) % NUM_AUDIO_BUFFS;

    // Generate next music
    Chunk chunk;
    onGetData(chunk);

    // Apply reverb
    applyReverb(chunk.samples, chunk.sampleCount);

    memcpy(m_WaveOut[m_CurWaveOut], chunk.samples, chunk.sampleCount * sizeof(int16_t));

    // Release the lock
    ReleaseMutex(m_Mutex);
}

void WinAudio::applyReverb(int16_t* samples, size_t sampleCount) {
    for (size_t i = 0; i < sampleCount; i++) {
        int delayedIndex = (m_ReverbBufferIndex + m_ReverbBufferSize - (REVERB_DELAY_MS * m_SampleRate / 1000) * 2) % m_ReverbBufferSize;
        int16_t delayedSample = m_ReverbBuffer[delayedIndex];
        m_ReverbBuffer[m_ReverbBufferIndex] = samples[i] + delayedSample * REVERB_DECAY;
        samples[i] = std::min(std::max(samples[i] + delayedSample * REVERB_DECAY, -32768.0f), 32767.0f);
        m_ReverbBufferIndex = (m_ReverbBufferIndex + 1) % m_ReverbBufferSize;
    }
}

void CALLBACK WinAudio::Callback(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    // Only listen for end of block messages.
    if (uMsg != WOM_DONE) { return; }
    WIN_AUDIO->SubmitBuffer();
}
