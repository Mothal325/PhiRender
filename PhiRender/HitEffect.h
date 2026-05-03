#pragma once
#include "Constants.h"
#include "offchart.h"

constexpr float HIT_EFFECT_DURATION = 0.5f;			//打击效果持续时间
constexpr float HIT_EFFECT_PARTICLE = NW * 0.2f;	//打击效果粒子大小

struct HitEffect
{
	float x, y;
	float hittime;
	float duration;
	float xVel[4];
	float yVel[4];

	void Init(float Lx, float Ly, float Lr, float xPos, float time);
	void Draw(float time);
};

struct HoldingHold
{
	float positionX;
	int lineid;
	int hittime;
	int holdtime;
	int count;
	bool holding;

	void Init(const OFF::Notedata& note);
};

struct HitEffectManager
{
	void AddEffect(const OFF::Linedata& line, const OFF::Notedata& note, float time);
	void DrawHitEffect(float time);
	void UpdateHoldHitEffect(const std::vector<OFF::Linedata>& data, float time);
	void Clear(void);

private:
	std::vector<HitEffect> Effects;
	std::vector<HoldingHold> Holds;
};