#include <cmath>
#include <raylib.h>
#include "Constants.h"
#include "offchart.h"
#include "HitEffect.h"

void HitEffect::Init(float Lx, float Ly, float Lr, float xPos, float time)
{
	float theta = Lr / 180.0 * PI;
	x = std::cos(theta) * xPos * OFF_X * SW + Lx * SW;
	y = std::sin(theta) * xPos * OFF_X * SW + Ly * SH;
	y = SH - y;
	hittime = time;
	duration = HIT_EFFECT_DURATION;
	for (int i = 0; i < 4; i++)
	{
		theta = (float)GetRandomValue(1, 360) / 180.0f * PI;	//发射方向
		float vel = (float)GetRandomValue(100, 150) / 100.0f;	//发射速度？
		xVel[i] = std::cos(theta) * vel;
		yVel[i] = std::sin(theta) * vel;
	}
}

void HitEffect::Draw(float time)
{
	float rt = time - hittime;
	float size = (1.0f - powf((duration - rt) / duration, 3.0f)) * NW;
	unsigned char alpha = fmaxf(0.0f, 255.0f * (duration - rt) / duration);
	DrawRectangle(x - size / 2, y - size / 2, size, size, { 255, 204, 48, alpha });
	//粒子
	for (int i = 0; i < 4; i++)
	{
		float partx, party;
		partx = x + xVel[i] * size - HIT_EFFECT_PARTICLE / 2;
		party = y + yVel[i] * size - HIT_EFFECT_PARTICLE / 2;
		DrawRectangle(partx, party, HIT_EFFECT_PARTICLE, HIT_EFFECT_PARTICLE, { 255, 204, 48, alpha });
	}
}

void HoldingHold::Init(const OFF::Notedata &note)
{
	positionX = note.note.positionX;
	lineid = note.lineid;
	hittime = note.note.time;
	holdtime = note.note.holdTime;
	count = 0;
	holding = true;
}

void HitEffectManager::AddEffect(const std::vector<OFF::Linedata>& lines, std::vector<OFF::Notedata>& notes, float time)
{
	for(auto &note : notes)
	{
		int i = note.lineid;
		if (note.note.time <= time * lines[i].bpm / OFF_T && !note.isPlayed)
		{
			note.isPlayed = true;
			if (note.note.type == 3)
			{
				HoldingHold Hold;
				Hold.Init(note);
				Holds.push_back(Hold);
			}
			else
			{
				HitEffect effect;
				effect.Init(lines[i].x, lines[i].y, lines[i].r, note.note.positionX, note.note.time * OFF_T / lines[i].bpm);
				Effects.push_back(effect);
			}
		}
	}
}

void HitEffectManager::DrawHitEffect(float time)
{
	for (auto &effect : Effects)
	{
		effect.Draw(time);
	}

	Effects.erase(std::remove_if(Effects.begin(), Effects.end(),
		[&time](const HitEffect& e) {return time - e.hittime > e.duration; }), Effects.end());
}

void HitEffectManager::UpdateHoldHitEffect(std::vector<OFF::Linedata> data, float time)
{
	for (auto &Hold : Holds)
	{
		int i = Hold.lineid;
		float dt = OFF_T / data[i].bpm;
		float hittime = Hold.hittime * dt;
		float holdtime = Hold.holdtime * dt;
		if (time - hittime >= holdtime)
		{
			Hold.holding = false;
			continue;
		}
		if (time - hittime >= Hold.count * dt * 16)
		{
			HitEffect effect;
			effect.Init(data[i].x, data[i].y, data[i].r, Hold.positionX, time);
			Effects.push_back(effect);
			Hold.count++;
		}
	}

	Holds.erase(std::remove_if(Holds.begin(), Holds.end(),
		[](HoldingHold& h) {return !h.holding; }), Holds.end());
}

void HitEffectManager::clear()
{
	Effects.clear();
	Holds.clear();
}