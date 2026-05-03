#include <vector>
#include <cmath>
#include "offchart.h"
#include "Resource.h"
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

void DrawNote(const std::vector<OFF::Notedata>& notedata, const std::vector<OFF::Linedata>& data, float time, bool renderhold, const Resource& res)
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
		if (d > -0.002 && d < 2.0 / OFF_Y)
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
				switch (note.type + mhnumber)
				{
				case 1:
					DrawTexturePro(res.Tclick, { 0, 0, (float)res.Tclick.width, (float)res.Tclick.height }, { x, SH - y, NW, (float)res.Tclick.height / (float)res.Tclick.width * NW }, { NW / 2.0f, (float)res.Tclick.height / (float)res.Tclick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 2:
					DrawTexturePro(res.Tdrag, { 0, 0, (float)res.Tdrag.width, (float)res.Tdrag.height }, { x, SH - y, NW, (float)res.Tdrag.height / (float)res.Tdrag.width * NW }, { NW / 2.0f, (float)res.Tdrag.height / (float)res.Tdrag.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 4:
					DrawTexturePro(res.Tflick, { 0, 0, (float)res.Tflick.width, (float)res.Tflick.height }, { x, SH - y, NW, (float)res.Tflick.height / (float)res.Tflick.width * NW }, { NW / 2.0f, (float)res.Tflick.height / (float)res.Tflick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case (1 + 4):
					DrawTexturePro(res.Tclick_mh, { 0, 0, (float)res.Tclick_mh.width, (float)res.Tclick_mh.height }, { x, SH - y, (float)res.Tclick_mh.width / (float)res.Tclick.width * NW, (float)res.Tclick_mh.height / (float)res.Tclick.width * NW }, { (float)res.Tclick_mh.width / (float)res.Tclick.width * NW / 2.0f, (float)res.Tclick_mh.height / (float)res.Tclick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case (2 + 4):
					DrawTexturePro(res.Tdrag_mh, { 0, 0, (float)res.Tdrag_mh.width, (float)res.Tdrag_mh.height }, { x, SH - y, (float)res.Tdrag_mh.width / (float)res.Tdrag.width * NW, (float)res.Tdrag_mh.height / (float)res.Tdrag.width * NW }, { (float)res.Tdrag_mh.width / (float)res.Tdrag.width * NW / 2.0f, (float)res.Tdrag_mh.height / (float)res.Tdrag.width * NW / 2.0f }, rotation, WHITE);
					break;
				case (4 + 4):
					DrawTexturePro(res.Tflick_mh, { 0, 0, (float)res.Tflick_mh.width, (float)res.Tflick_mh.height }, { x, SH - y, (float)res.Tflick_mh.width / (float)res.Tflick.width * NW, (float)res.Tflick_mh.height / (float)res.Tflick.width * NW }, { (float)res.Tflick_mh.width / (float)res.Tflick.width * NW / 2.0f, (float)res.Tflick_mh.height / (float)res.Tflick.width * NW / 2.0f }, rotation, WHITE);
					break;
				}
			}
			else if (note.type == 3 && note.speed != 0 && renderhold)
			{
				float length = note.speed * note.holdTime * OFF_T / data[id].bpm * OFF_Y * SH;
				float remainlength = length;
				if (note.time < t && t <= note.time + note.holdTime)
					remainlength -= note.speed * (t - note.time) * OFF_T / data[id].bpm * OFF_Y * SH;
				if (notedata[i].ismh)
				{
					float holdmhwidth = (float)res.Thold_mh.width / (float)res.Thold.width * NW;
					DrawTexturePro(res.Thold_mh, { 0, (float)res.holdAtlasMH[0], (float)res.Thold_mh.width, (float)res.Thold_mh.height - res.holdAtlasMH[1] - res.holdAtlasMH[0] }, { x, SH - y, holdmhwidth, remainlength }, { holdmhwidth / 2.0f, remainlength }, rotation + 180.0 * updown, WHITE);
					DrawTexturePro(res.Thold_mh, { 0, 0, (float)res.Thold_mh.width, (float)res.holdAtlasMH[0] }, { x, SH - y, holdmhwidth, (float)res.holdAtlasMH[0] / (float)res.Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, remainlength + (float)res.holdAtlasMH[0] / (float)res.Thold_mh.width * holdmhwidth }, rotation + 180.0 * updown, WHITE);
					if (note.time > t)
					{
						DrawTexturePro(res.Thold_mh, { 0, (float)res.Thold_mh.height - res.holdAtlasMH[1], (float)res.Thold_mh.width, (float)res.holdAtlasMH[1] }, { x, SH - y, holdmhwidth, (float)res.holdAtlasMH[1] / (float)res.Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, 0.0f }, rotation + 180.0 * updown, WHITE);
					}
				}
				else
				{
					DrawTexturePro(res.Thold, { 0, (float)res.holdAtlas[0], (float)res.Thold.width, (float)res.Thold.height - res.holdAtlas[1] - res.holdAtlas[0] }, { x, SH - y, NW, remainlength }, { NW / 2.0f, remainlength }, rotation + 180.0 * updown, WHITE);
					DrawTexturePro(res.Thold, { 0, 0, (float)res.Thold.width, (float)res.holdAtlas[0] }, { x, SH - y, NW, (float)res.holdAtlas[0] / (float)res.Thold.width * NW }, { NW / 2.0f, remainlength + (float)res.holdAtlas[0] / (float)res.Thold.width * NW }, rotation + 180.0 * updown, WHITE);
					if (note.time > t)
					{
						DrawTexturePro(res.Thold, { 0, (float)res.Thold.height - res.holdAtlas[1], (float)res.Thold.width, (float)res.holdAtlas[1] }, { x, SH - y, NW, (float)res.holdAtlas[1] / (float)res.Thold.width * NW }, { NW / 2.0f, 0.0f }, rotation + 180.0 * updown, WHITE);

					}
				}
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

void State::Draw(float time, const Resource& res)
{
	DrawJudgeLine(linedata);

	DrawNote(notedata, linedata, time, 1, res);
	DrawNote(notedata, linedata, time, 0, res);
}