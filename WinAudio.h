#pragma once
#define WIN32_LEAN_AND_MEAN // Reduce compile time of windows.h
#include <windows.h>
#include <mmsystem.h>
#include <cstdint>
#include <vector>
#undef min
#undef max

class WinAudio {
public:
    static const int NUM_AUDIO_BUFFS = 5;
    static const int AUDIO_BUFF_SIZE = 4096;
    static WinAudio* WIN_AUDIO;

    struct Chunk {
        int16_t* samples;
        size_t sampleCount;
    };

    WinAudio(HWND handle, int sample_rate);
    ~WinAudio();

    bool play();
    bool stop();
    virtual bool onGetData(Chunk& data) = 0;

protected:
    static void CALLBACK Callback(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
    void SubmitBuffer();
    void applyReverb(int16_t* samples, size_t sampleCount);

    std::vector<int16_t> m_ReverbBuffer;
    size_t m_ReverbBufferSize;
    size_t m_ReverbBufferIndex;

    // Wave properties
    HWAVEOUT m_HWaveOut;
    HANDLE m_Mutex;
    WAVEFORMATEX m_Format;
    WAVEHDR m_WaveOutHdr[NUM_AUDIO_BUFFS];
    int16_t m_WaveOut[NUM_AUDIO_BUFFS][AUDIO_BUFF_SIZE];
    int m_CurWaveOut;
    int m_SampleRate;
    bool m_IsReleasing;
};


