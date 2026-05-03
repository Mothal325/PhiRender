#pragma once
#include <vector>
#include "offchart.h"
#include "Resource.h"
#include "HitEffect.h"
#include "HitSound.h"

struct State
{
	void Init(const OFF::Chartdata& data);
	void Reset(void);
	void Update(float time, const std::vector<OFF::judgeLine>& lines, HitEffectManager& EffectM, HitSoundManager& SoundM);
	void Draw(float time, const Resource& res);
	int GetNoteNum(void) { return notenum; }
	int GetHitNum(void) { return hitnum; }

private:
	std::vector<OFF::Notedata> notedata;
	std::vector<OFF::Linedata> linedata;
	int hitnum = 0;
	int notenum = 0;
};