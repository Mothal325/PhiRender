#pragma once
#include <format>
#include <string>
#include <vector>
#include "include\raylib.h"
#include "offchart.h"

constexpr size_t SAMPLE_RATE = 48000;
constexpr size_t SAMPLE_SIZE = 32;
constexpr size_t CHANNELS = 2;

std::vector<float> AudioToFloat(std::string name)
{
	Wave wave = LoadWave(name.c_str());
	WaveFormat(&wave, SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
	float* samples = LoadWaveSamples(wave);
	std::vector<float> result(samples, samples + wave.frameCount * wave.channels);
	UnloadWaveSamples(samples);
	UnloadWave(wave);
	return result;
}

void MixMusic(const OFF::Chartdata& data, const std::string& songname)
{
	std::vector<float> song = AudioToFloat(songname);
	std::vector<float> click = AudioToFloat("res/click.ogg");
	std::vector<float> drag = AudioToFloat("res/drag.ogg");
	std::vector<float> flick = AudioToFloat("res/flick.ogg");

	std::vector<OFF::Notedata> notes = OFF::ReadNotedata(data);
	for (auto& n : notes)
	{
		float time = n.note.time * OFF_T / data.lines[n.lineid].bpm;
		size_t start_frame = (size_t)(time * SAMPLE_RATE) * CHANNELS;
		std::vector<float>* sound = &click;
		switch (n.note.type)
		{
		case 2: sound = &drag; break;
		case 4: sound = &flick; break;
		default: break;
		}
		for (size_t i = 0; i < sound->size() && start_frame + i < song.size(); i++)
		{
			song[start_frame + i] += (*sound)[i];
		}
	}

	std::string cmd = std::format(
		"ffmpeg -y -loglevel error -f f32le -ar {} -ac {} -i - "
		"-af \"alimiter=limit=0.95\" "
		"-c:a aac -b:a 128k temp.m4a",
		SAMPLE_RATE, CHANNELS);
	FILE* pipe = _popen(cmd.c_str(), "wb");
	fwrite(song.data(), sizeof(float), song.size(), pipe);
	_pclose(pipe);
}