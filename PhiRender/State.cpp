#include <vector>
#include <cmath>
#include "offchart.h"
#include "NoteTexture.h"
#include "HitEffect.h"
#include "HitSound.h"
#include "State.h"

void State::Init(const OFF::Chartdata& data)
{
	linedata.resize(data.lines.size());
	notedata = OFF::ReadNotedata(data);
	notenum = notedata.size();
}

void State::Reset(void)
{
	for (auto& ld : linedata)
	{
		ld.index[0] = 0;
		ld.index[1] = 0;
		ld.index[2] = 0;
		ld.index[3] = 0;
	}
	for (auto& nd : notedata)
	{
		nd.isPlayed = false;
	}
	hitnum = 0;
}

void State::Update(float time, const std::vector<OFF::judgeLine>& lines, HitEffectManager& EffectM, HitSoundManager& SoundM)
{
	hitnum = 0;
	for (int i = 0; i < linedata.size(); i++)
	{
		linedata[i].FindLine(lines[i], time);
	}
	for (int i = 0; i < notedata.size(); i++)
	{
		float t = time * lines[notedata[i].lineid].bpm / OFF_T;
		if (notedata[i].note.time <= t && !notedata[i].isPlayed)
		{
			notedata[i].isPlayed = true;
			SoundM.PlayHitSound(i);
			EffectM.AddEffect(linedata[notedata[i].lineid], notedata[i], time);
		}
		if (notedata[i].note.time + notedata[i].note.holdTime < t)
		{
			hitnum++;
		}
	}
	EffectM.UpdateHoldHitEffect(linedata, time);
}

void DrawNote(const std::vector<OFF::Notedata>& notedata, const std::vector<OFF::Linedata>& data, float time, bool renderhold, NoteTexture& res)
{
	for (int i = notedata.size() - 1; i >= 0; i--)
	{
		OFF::Note note = notedata[i].note;
		bool updown = !notedata[i].isAbove;
		int id = notedata[i].lineid;
		float t = time * data[id].bpm / OFF_T;
		int mhnumber = notedata[i].ismh ? 4 : 0;
		if (note.time + note.holdTime <= t)
		{
			continue;
		}
		float d = note.floorPosition - data[id].f;
		if (d > -0.002 && d < 2.0 / OFF_Y || t >= note.time && t < note.time + note.holdTime)
		{
			float x, y, lx, ly, theta;
			lx = note.positionX * OFF_X * SW;
			ly = d * OFF_Y * SH * (updown ? -1.0 : 1.0) * (note.type == 3 ? 1.0 : note.speed);
			if (note.time < t && t <= note.time + note.holdTime)
			{
				ly = 0;
			}
			theta = data[id].r / 180.0 * PI;
			x = std::cos(theta) * lx - std::sin(theta) * ly + data[id].x * SW;
			y = std::sin(theta) * lx + std::cos(theta) * ly + data[id].y * SH;
			float rotation = -data[id].r;
			if (note.type != 3 && !renderhold)
			{
				res.DrawNoteTexture(note.type, notedata[i].ismh, x, y, rotation);
			}
			else if (note.type == 3 && note.speed != 0 && renderhold)
			{
				float length = note.speed * note.holdTime * OFF_T / data[id].bpm * OFF_Y * SH;
				float remainlength = length;
				if (note.time < t && t <= note.time + note.holdTime)
					remainlength -= note.speed * (t - note.time) * OFF_T / data[id].bpm * OFF_Y * SH;
				res.DrawHoldTexture(note.time <= t, notedata[i].ismh, x, y, rotation, remainlength, updown);
			}
		}
	}
}

void DrawJudgeLine(const std::vector<OFF::Linedata>& data)
{
	for (auto& line : data)
	{
		Color c = { 255, 255, 255, line.a * 255 };
		Rectangle l = { line.x * SW, (1.0 - line.y) * SH, LINEL * SH, LINEW * SH };
		DrawRectanglePro(l, { l.width / 2, l.height / 2 }, -line.r, c);
	}
}

void State::Draw(float time, NoteTexture& res)
{
	DrawJudgeLine(linedata);

	DrawNote(notedata, linedata, time, 1, res);
	DrawNote(notedata, linedata, time, 0, res);
}