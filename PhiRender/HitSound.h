#pragma once
#include <raylib.h>
#include "offchart.h"

struct HitSoundManager
{
	void Init(const OFF::Chartdata& data);
	void Free(void);
	void SetHitSoundVolume(float volume);
	void PlayHitSound(int index);

private:
	Sound Sclick, Sdrag, Sflick;
	std::vector<Sound> Snotes;
};