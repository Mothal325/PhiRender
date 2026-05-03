#include <raylib.h>
#include "offchart.h"
#include "HitSound.h"

void HitSoundManager::Init(const OFF::Chartdata& data)
{
	Sclick = LoadSound("res/click.ogg");
	Sdrag = LoadSound("res/drag.ogg");
	Sflick = LoadSound("res/flick.ogg");
	std::vector<OFF::Notedata> notedata = OFF::ReadNotedata(data);
	for (auto& note : notedata)
	{
		switch (note.note.type)
		{
		case 1: //Tap和Hold共用音效
		case 3: Snotes.push_back(LoadSoundAlias(Sclick)); break;
		case 2: Snotes.push_back(LoadSoundAlias(Sdrag));  break;
		case 4: Snotes.push_back(LoadSoundAlias(Sflick)); break;
		}
	}
}

void HitSoundManager::Free(void)
{
	for (auto& hitsound : Snotes)
	{
		UnloadSoundAlias(hitsound);
	}
	UnloadSound(Sclick);
	UnloadSound(Sdrag);
	UnloadSound(Sflick);
}

void HitSoundManager::SetHitSoundVolume(float volume)
{
	SetSoundVolume(Sclick, volume);
	SetSoundVolume(Sdrag, volume);
	SetSoundVolume(Sflick, volume);
}

void HitSoundManager::PlayHitSound(int index)
{
	PlaySound(Snotes[index]);
}