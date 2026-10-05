#include <vector>
#include <cmath>
#include "offchart.h"
#include "NoteTexture.h"
#include "HitEffect.h"
#include "HitSound.h"
#include "State.h"

#include "include/rlgl.h"

void State::Init(const OFF::Chartdata& data)
{
	linedata.resize(data.lines.size());
	notedata = OFF::ReadNotedata(data);
	blockdata.resize(data.blockAreaList.size());
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

static void DrawNote(const std::vector<OFF::Notedata>& notedata, const std::vector<OFF::Linedata>& data, float time, bool renderhold, NoteTexture& res)
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
			float dt = std::max(note.time - t, 0.0f);
			float scale = 1.0f - std::pow(std::max(0.0f, (dt - 16.0f) / 32.0f), 2);
			theta = data[id].r / 180.0 * PI;
			x = std::cos(theta) * lx - std::sin(theta) * ly + data[id].x * SW;
			y = std::sin(theta) * lx + std::cos(theta) * ly + data[id].y * SH;
			float rotation = -data[id].r;
			if (note.type != 3 && !renderhold)
			{
				res.DrawNoteTexture(note.type, notedata[i].ismh, x, y, rotation, scale * 1.5f);
			}
			else if (note.type == 3 && note.speed != 0 && renderhold)
			{
				float length = note.speed * note.holdTime * OFF_T / data[id].bpm * OFF_Y * SH;
				float remainlength = length;
				if (note.time < t && t <= note.time + note.holdTime)
					remainlength -= note.speed * (t - note.time) * OFF_T / data[id].bpm * OFF_Y * SH;
				res.DrawHoldTexture(note.time <= t, notedata[i].ismh, x, y, rotation, remainlength, updown, scale * 1.5f);
			}
		}
	}
}

static void DrawJudgeLine(const std::vector<OFF::Linedata>& data)
{
	for (auto& line : data)
	{
		Color c = { 255, 255, 255, line.a * 255 };
		Rectangle l = { line.x * SW, (1.0 - line.y) * SH, LINEL * SH, LINEW * SH };
		DrawRectanglePro(l, { l.width / 2, l.height / 2 }, -line.r, c);
	}
}

void State::UpdateBlock(float time, const std::vector<OFF::BlockArea>& blocks)
{
	for (int i = 0; i < blockdata.size(); i++)
	{
		blockdata[i].FindBlock(blocks[i], time);
	}
}

static void DrawBlockArea(const std::vector<OFF::Blockdata>& data, int mode) //0 -> pin, 1 -> pos & neg
{
	static RenderTexture2D canvas = LoadRenderTexture(SW, SH);
	static RenderTexture2D canvaspin = LoadRenderTexture(SW, SH);

	if (mode == 0)
	{
		BeginTextureMode(canvaspin);
		ClearBackground(BLANK);
		for (int i = 0; i < data.size(); i++)
		{
			if (data[i].state == 0 || data[i].state == 2 || data[i].state == 6)
			{
				continue;
			}
			DrawRectanglePro({ data[i].x, data[i].y, data[i].w, data[i].h },
				{ data[i].w / 2.0f, data[i].h / 2.0f }, data[i].rotation, { 255, 255, 64, 255 });
		}
		EndTextureMode();
		DrawTexture(canvaspin.texture, 0, 0, { 255, 255, 255, 64 });
	}
	else
	{
		BeginTextureMode(canvas);
		ClearBackground(BLANK);
		for (int i = 0; i < data.size(); i++)
		{
			if (data[i].state != 2)
			{
				continue;
			}
			DrawRectanglePro({ data[i].x, data[i].y, data[i].w, data[i].h },
				{ data[i].w / 2.0f, data[i].h / 2.0f }, data[i].rotation, { 255, 0, 0, 255 });
		}

		rlSetBlendFactors(RL_ONE_MINUS_DST_COLOR, RL_ONE_MINUS_SRC_COLOR, RL_FUNC_ADD);
		BeginBlendMode(BLEND_CUSTOM);
		for (int i = 0; i < data.size(); i++)
		{
			if (data[i].state != 6)
			{
				continue;
			}
			DrawRectanglePro({ data[i].x, data[i].y, data[i].w, data[i].h },
				{ data[i].w / 2.0f, data[i].h / 2.0f }, data[i].rotation, { 255, 0, 0, 255 });
		}
		EndBlendMode();
		EndTextureMode();

		DrawTexture(canvas.texture, 0, 0, { 255, 255, 255, 64 });
	}
}

void State::Draw(float time, NoteTexture& res) const
{
	DrawBlockArea(blockdata, 0);
	DrawJudgeLine(linedata);
	DrawNote(notedata, linedata, time, 1, res);
	DrawNote(notedata, linedata, time, 0, res);
	DrawBlockArea(blockdata, 1);
}