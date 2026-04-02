#include "VSCore.h"
#pragma comment(lib, "winmm.lib")


void play_sound(short* buffer, int samples)
{
	HWAVEOUT hWaveOut;
	WAVEFORMATEX wfx = {
		.wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = 1,
		.nSamplesPerSec = 16000,
		.wBitsPerSample = 16,
		.nBlockAlign = 2,
		.nAvgBytesPerSec = 16000 * 2
	};

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

	WAVEHDR header = { 0 };
	header.lpData = (char*)buffer;
	header.dwBufferLength = samples * sizeof(short);
	waveOutPrepareHeader(hWaveOut, &header, sizeof(header));
	waveOutWrite(hWaveOut, &header, sizeof(header));

	while (waveOutUnprepareHeader(hWaveOut, &header, sizeof(header)) == WAVERR_STILLPLAYING) {
		Sleep(10);
	}

	waveOutClose(hWaveOut);
}
