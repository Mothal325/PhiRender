#pragma once
#include <vector>
#include "offchart.h"
#include "NoteTexture.h"
#include "HitEffect.h"
#include "HitSound.h"

struct State
{
	void Init(const OFF::Chartdata& data);
	void Reset(void);
	void Update(float time, const std::vector<OFF::judgeLine>& lines, HitEffectManager& EffectM, HitSoundManager& SoundM);
	void UpdateBlock(float time, const std::vector<OFF::BlockArea>& blocks);
	void Draw(float time, NoteTexture& res) const;
	int GetNoteNum(void) const { return notenum; }
	int GetHitNum(void) const { return hitnum; }

private:
	std::vector<OFF::Notedata> notedata;
	std::vector<OFF::Linedata> linedata;
	std::vector<OFF::Blockdata> blockdata;
	int hitnum = 0;
	int notenum = 0;
};