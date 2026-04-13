#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <raylib.h>
#include "rpechart.h"
#include "offchart.h"

#define SW 1920
#define SH 1080
#define NW (SW * 0.12f)
#define LINEL 5.75f
#define LINEW 0.0075f
#define BGMSPEED 1.0f

struct HitEffect
{
	float x = 0, y = 0;
	float hittime = 0;
	float duration = 0.5;

	void Init(float Lx, float Ly, float Lr, float xPos, float time)
	{
		float theta = Lr / 180.0 * PI;
		x = std::cos(theta) * xPos * OFF_X * SW + Lx * SW;
		y = std::sin(theta) * xPos * OFF_X * SW + Ly * SH;
		y = SH - y;
		hittime = time;
	}

	void Draw(float time)
	{
		float rt = time - hittime;
		float size = (1.0f - powf((duration - rt) / duration, 3.0f)) * NW;
		unsigned char alpha = fmaxf(0.0f, 255.0f * (duration - rt) / duration);
		DrawRectangle(x - size / 2, y - size / 2, size, size, {255, 204, 48, alpha });
	}
};

struct HoldingHold
{
	float positionX;
	int lineid;
	int hittime;
	int holdtime;
	int count;
	bool holding;

	void Init(OFF::Notedata data)
	{
		positionX = data.note.positionX;
		lineid = data.lineid;
		hittime = data.note.time;
		holdtime = data.note.holdTime;
		count = 1;
		holding = true;
	}
};

Texture Tclick, Tclick_mh, Tdrag, Tdrag_mh, Tflick, Tflick_mh, Thold, Thold_mh;
Sound Sclick, Sdrag, Sflick;
std::vector<Sound> Snotes;
std::vector<HitEffect> Effects;
std::vector<HoldingHold> Holds;
int holdAtlas[2] = { 50, 50 }, holdAtlasMH[2] = { 50, 50 };

void DrawHitEffect(float time)
{
	for (HitEffect effect : Effects)
	{
		effect.Draw(time);
	}

	Effects.erase(std::remove_if(Effects.begin(), Effects.end(),
		[&time](const HitEffect &e) {return time - e.hittime > e.duration; }), Effects.end());
}

void UpdateHoldHitEffect(std::vector<OFF::Linedata> data, float time)
{
	for (int i = 0; i < Holds.size(); i++)
	{
		int id = Holds[i].lineid;
		float dt = OFF_T / data[id].bpm;
		float hittime = Holds[i].hittime * dt;
		float holdtime = Holds[i].holdtime * dt;
		if (time - hittime >= holdtime)
		{
			Holds[i].holding = false;
			continue;
		}
		if (time - hittime >= Holds[i].count * dt * 16)
		{
			HitEffect effect;
			effect.Init(data[id].x, data[id].y, data[id].r, Holds[i].positionX, time);
			Effects.push_back(effect);
			Holds[i].count++;
		}
	}

	Holds.erase(std::remove_if(Holds.begin(), Holds.end(),
		[](HoldingHold& h) {return !h.holding; }), Holds.end());
}

void InitNoteSound(std::vector<OFF::Notedata> notedata)
{
	for (int i = 0; i < notedata.size(); i++)
	{
		switch (notedata[i].note.type)
		{
		case 1:
		case 3:
			Snotes.push_back(LoadSoundAlias(Sclick));
			break;
		case 2:
			Snotes.push_back(LoadSoundAlias(Sdrag));
			break;
		case 4:
			Snotes.push_back(LoadSoundAlias(Sflick));
			break;
		default:
			break;
		}
	}
}

void FreeNoteSound(void)
{
	for (int i = 0; i < Snotes.size(); i++)
	{
		UnloadSoundAlias(Snotes[i]);
	}
}

std::vector<OFF::Notedata> ReadNotedata(OFF::Chartdata data)
{
	std::vector<OFF::Notedata> notedata;
	std::vector<int> hitnum;
	std::vector<int> hittime;
	for (int i = 0; i < data.lines.size(); i++)
	{
		OFF::judgeLine aline = data.lines[i];
		for (int j = 0; j < aline.notesAbove.size(); j++)
		{
			OFF::Note anote = aline.notesAbove[j];
			OFF::Notedata noted;
			noted.note = anote;
			noted.lineid = i;
			noted.isAbove = true;
			noted.ismh = false;
			bool findit = false;
			for (int k = 0; k < hittime.size(); k++)
			{
				if (hittime[k] == anote.time)
				{
					findit = true;
					hitnum[k]++;
					break;
				}
			}
			if (findit == false)
			{
				hittime.push_back(anote.time);
				hitnum.push_back(1);
			}
			notedata.push_back(noted);
		}
		for (int j = 0; j < aline.notesBelow.size(); j++)
		{
			OFF::Note anote = aline.notesBelow[j];
			OFF::Notedata noted;
			noted.note = anote;
			noted.lineid = i;
			noted.isAbove = false;
			bool findit = false;

			for (int k = 0; k < hittime.size(); k++)
			{
				if (hittime[k] == anote.time)
				{
					findit = true;
					hitnum[k]++;
					break;
				}
			}
			if (findit == false)
			{
				hittime.push_back(anote.time);
				hitnum.push_back(1);
			}
			notedata.push_back(noted);
		}
	}
	for (int i = 0; i < notedata.size(); i++)
	{
		OFF::Note anote = notedata[i].note;
		for (int j = 0; j < hittime.size(); j++)
		{
			if (hittime[j] == anote.time)
			{
				if (hitnum[j] > 1)
				{
					notedata[i].ismh = true;
				}
			}
		}
	}
	
	return notedata;
}

int DrawOFFNote(std::vector<OFF::Notedata> &notedata, std::vector<OFF::Linedata> data, float time, bool renderhold)
{
	int hitnum = 0;
	char debugtext[64];
	for (int i = notedata.size() - 1; i >= 0; i--)
	{
		OFF::Note note = notedata[i].note;
		bool updown = !notedata[i].isAbove;
		int id = notedata[i].lineid;
		float t = time * data[id].bpm / OFF_T;
		int mhnumber = notedata[i].ismh ? 4 : 0;
		if (note.time <= t && !renderhold)
		{
			if (!notedata[i].isPlayed)
			{
				notedata[i].isPlayed = true;
				PlaySound(Snotes[i]);
				HitEffect effect;
				effect.Init(data[id].x, data[id].y, data[id].r, note.positionX, time);
				Effects.push_back(effect);
				if (note.type == 3)
				{
					HoldingHold hold;
					hold.Init(notedata[i]);
					Holds.push_back(hold);
				}
			}
			if (note.type != 3)
			{
				hitnum++;
				continue;
			}
			else if (note.time + note.holdTime <= t)
			{
				hitnum++;
				continue;
			}
		}
		float d = note.floorPosition - data[id].f;
		if (d > -0.002 && d < 2.0 / OFF_Y || note.time < t && t <= note.time + note.holdTime)
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
					DrawTexturePro(Tclick, { 0, 0, (float)Tclick.width, (float)Tclick.height }, { x, SH - y, NW, (float)Tclick.height / (float)Tclick.width * NW }, { NW / 2.0f, (float)Tclick.height / (float)Tclick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 2:
					DrawTexturePro(Tdrag, { 0, 0, (float)Tdrag.width, (float)Tdrag.height }, { x, SH - y, NW, (float)Tdrag.height / (float)Tdrag.width * NW }, { NW / 2.0f, (float)Tdrag.height / (float)Tdrag.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 4:
					DrawTexturePro(Tflick, { 0, 0, (float)Tflick.width, (float)Tflick.height }, { x, SH - y, NW, (float)Tflick.height / (float)Tflick.width * NW }, { NW / 2.0f, (float)Tflick.height / (float)Tflick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 5:
					DrawTexturePro(Tclick_mh, { 0, 0, (float)Tclick_mh.width, (float)Tclick_mh.height }, { x, SH - y, (float)Tclick_mh.width / (float)Tclick.width * NW, (float)Tclick_mh.height / (float)Tclick.width * NW }, { (float)Tclick_mh.width / (float)Tclick.width * NW / 2.0f, (float)Tclick_mh.height / (float)Tclick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 6:
					DrawTexturePro(Tdrag_mh, { 0, 0, (float)Tdrag_mh.width, (float)Tdrag_mh.height }, { x, SH - y, (float)Tdrag_mh.width / (float)Tdrag.width * NW, (float)Tdrag_mh.height / (float)Tdrag.width * NW }, { (float)Tdrag_mh.width / (float)Tdrag.width * NW / 2.0f, (float)Tdrag_mh.height / (float)Tdrag.width * NW / 2.0f }, rotation, WHITE);
					break;
				case 8:
					DrawTexturePro(Tflick_mh, { 0, 0, (float)Tflick_mh.width, (float)Tflick_mh.height }, { x, SH - y, (float)Tflick_mh.width / (float)Tflick.width * NW, (float)Tflick_mh.height / (float)Tflick.width * NW }, { (float)Tflick_mh.width / (float)Tflick.width * NW / 2.0f, (float)Tflick_mh.height / (float)Tflick.width * NW / 2.0f }, rotation, WHITE);
					break;
				default:
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
					float holdmhwidth = (float)Thold_mh.width / (float)Thold.width * NW;
					DrawTexturePro(Thold_mh, { 0, (float)holdAtlasMH[0], (float)Thold_mh.width, (float)Thold_mh.height - holdAtlasMH[1] - holdAtlasMH[0] }, { x, SH - y, holdmhwidth, remainlength }, { holdmhwidth / 2.0f, remainlength }, rotation + 180.0 * updown, WHITE);
					DrawTexturePro(Thold_mh, { 0, 0, (float)Thold_mh.width, (float)holdAtlasMH[0] }, { x, SH - y, holdmhwidth, (float)holdAtlasMH[0] / (float)Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, remainlength + (float)holdAtlasMH[0] / (float)Thold_mh.width * holdmhwidth }, rotation + 180.0 * updown, WHITE);
					if (note.time > t)
					{
						DrawTexturePro(Thold_mh, { 0, (float)Thold_mh.height - holdAtlasMH[1], (float)Thold_mh.width, (float)holdAtlasMH[1] }, { x, SH - y, holdmhwidth, (float)holdAtlasMH[1] / (float)Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, 0.0f }, rotation + 180.0 * updown, WHITE);
					}
				}
				else
				{
					DrawTexturePro(Thold, { 0, (float)holdAtlas[0], (float)Thold.width, (float)Thold.height - holdAtlas[1] - holdAtlas[0] }, { x, SH - y, NW, remainlength }, { NW / 2.0f, remainlength }, rotation + 180.0 * updown, WHITE);
					DrawTexturePro(Thold, { 0, 0, (float)Thold.width, (float)holdAtlas[0] }, { x, SH - y, NW, (float)holdAtlas[0] / (float)Thold.width * NW }, { NW / 2.0f, remainlength + (float)holdAtlas[0] / (float)Thold.width * NW }, rotation + 180.0 * updown, WHITE);
					if (note.time > t)
					{
						DrawTexturePro(Thold, { 0, (float)Thold.height - holdAtlas[1], (float)Thold.width, (float)holdAtlas[1] }, { x, SH - y, NW, (float)holdAtlas[1] / (float)Thold.width * NW }, { NW / 2.0f, 0.0f }, rotation + 180.0 * updown, WHITE);

					}
				}
			}
		}
	}
	return hitnum;
}

void DrawOFFJudgeLine(OFF::judgeLine line, float time, OFF::Linedata &data)
{
	OFF::FindLine(line, time, data);
	Color c = { 255, 255, 255, data.a * 255 };
	Rectangle l = { data.x * SW, (1.0 - data.y) * SH, LINEL * SH, LINEW * SH };
	DrawRectanglePro(l, { l.width / 2, l.height / 2 }, -data.r, c);
}

int main(void)
{
	std::string chartname, songname, backgroundname;
	std::cout << "谱面文件路径：";
	std::getline(std::cin, chartname);
	
	OFF::Chartdata data = OFF::Readdata(chartname);
	std::vector<OFF::Notedata> notedata = ReadNotedata(data);

	std::cout << "音频文件路径：";
	std::getline(std::cin, songname);
	InitAudioDevice();
	Music bgm = LoadMusicStream(songname.c_str());

	std::cout << "背景文件路径：";
	std::getline(std::cin, backgroundname);

	SetTraceLogLevel(LOG_NONE);
	InitWindow(SW, SH, "Mothal's Phigros Render");
	Texture bg = LoadTexture(backgroundname.c_str());
	
	Font Phifont_s = LoadFontEx("res/Phifont.ttf", SH * 0.025, NULL, 0);
	Font Phifont_m = LoadFontEx("res/Phifont.ttf", SH * 0.05, NULL, 0);
	Font Phifont_l = LoadFontEx("res/Phifont.ttf", SH * 0.075, NULL, 0);
	Tclick = LoadTexture("res/click.png");
	Tclick_mh = LoadTexture("res/click_mh.png");
	Tdrag = LoadTexture("res/drag.png");
	Tdrag_mh = LoadTexture("res/drag_mh.png");
	Tflick = LoadTexture("res/flick.png");
	Tflick_mh = LoadTexture("res/flick_mh.png");
	Thold = LoadTexture("res/hold.png");
	Thold_mh = LoadTexture("res/hold_mh.png");
	Sclick = LoadSound("res/click.ogg");
	Sdrag = LoadSound("res/drag.ogg");
	Sflick = LoadSound("res/flick.ogg");
	InitNoteSound(notedata);
	SetSoundVolume(Sclick, 0.5f);
	SetSoundVolume(Sdrag, 0.5f);
	SetSoundVolume(Sflick, 0.5f);
	SetMusicVolume(bgm, 1.0f);
	SetMusicPitch(bgm, BGMSPEED);
	SetTextureFilter(bg, TEXTURE_FILTER_TRILINEAR);
	
	SetTargetFPS(0);
	bool isPlaying = false;
	bool shouldPlaying = false;
	char timetext[64] = "";
	char combotext[64] = "";
	char scoretext[64] = "";
	int notenum = 0, hitnum = 0;

	for (int i = 0; i < data.lines.size(); i++)
	{
		notenum += data.lines[i].notesAbove.size() + data.lines[i].notesBelow.size();
	}

	std::vector<OFF::Linedata> linedata;
	linedata.resize(data.lines.size());
	while (!WindowShouldClose())
	{

		if (IsKeyPressed(KEY_A))
		{
			TakeScreenshot("screenshot.png");
		}

		if (GetMusicTimeLength(bgm) - GetMusicTimePlayed(bgm) < 0.1f)
		{
			StopMusicStream(bgm);
			isPlaying = false;
			shouldPlaying = false;
			for (int i = 0; i < linedata.size(); i++)
			{
				linedata[i].index[0] = 0;
				linedata[i].index[1] = 0;
				linedata[i].index[2] = 0;
				linedata[i].index[3] = 0;
			}
			for (int i = 0; i < notedata.size(); i++)
			{
				notedata[i].isPlayed = false;
			}
		}

		if (IsKeyPressed(KEY_SPACE))
		{
			shouldPlaying = !shouldPlaying;
			if (shouldPlaying && !isPlaying)
			{
				PlayMusicStream(bgm);
				isPlaying = true;
			}
			else if (!shouldPlaying && isPlaying)
			{
				PauseMusicStream(bgm);
			}
			else if (shouldPlaying && isPlaying)
			{
				ResumeMusicStream(bgm);
			}
		}

		UpdateMusicStream(bgm);

		BeginDrawing();

		ClearBackground(BLACK);
		
		DrawTexturePro(bg, { 0, 0, (float)bg.width, (float)bg.height }, { (SW - (float)SH / bg.height * bg.width) / 2, 0, (float)SH / bg.height * bg.width, SH }, { 0, 0 }, 0, {128, 128, 128, 255});
		float playtime = GetMusicTimePlayed(bgm);
		for (int i = 0; i < data.lines.size(); i++)
		{
			DrawOFFJudgeLine(data.lines[i], playtime, linedata[i]);
		}

		hitnum = DrawOFFNote(notedata, linedata, playtime, true);
		hitnum = DrawOFFNote(notedata, linedata, playtime, false);

		UpdateHoldHitEffect(linedata, playtime);
		DrawHitEffect(playtime);

		sprintf_s(timetext, "Time:%.1f/%.1f\nFPS:%d", GetMusicTimePlayed(bgm), GetMusicTimeLength(bgm), GetFPS());
		sprintf_s(combotext, "%d", hitnum);
		sprintf_s(scoretext, "%07d", (int)((double)hitnum / notenum * 1000000.0 + 0.5));
		DrawTextEx(Phifont_s, timetext, { 0.0f, 0.0f }, SH * 0.025f, 0.0f, WHITE);
		DrawTextEx(Phifont_l, combotext, { (SW - MeasureTextEx(Phifont_l, combotext, SH * 0.075f, 0).x) / 2.0f, SH * 0.015f }, SH * 0.075f, 0.0f, WHITE);
		DrawTextEx(Phifont_s, "COMBO", { (SW - MeasureTextEx(Phifont_s, "COMBO", SH * 0.025f, 0).x) / 2.0f, SH * 0.1f }, SH * 0.025f, 0.0f, WHITE);
		DrawTextEx(Phifont_m, scoretext, { (float)SW - MeasureTextEx(Phifont_m, scoretext, SH * 0.05f, 0.72f).x - SH * 0.04f, SH * 0.03f }, SH * 0.05f, 0.72f, WHITE);

		EndDrawing();

	}
	
	UnloadMusicStream(bgm);
	FreeNoteSound();
	UnloadSound(Sclick);
	UnloadSound(Sdrag);
	UnloadSound(Sflick);
	UnloadTexture(bg);
	UnloadTexture(Tclick);
	UnloadTexture(Tdrag);
	UnloadTexture(Tflick);
	UnloadTexture(Tclick_mh);
	UnloadTexture(Tdrag_mh);
	UnloadTexture(Tflick_mh);
	UnloadFont(Phifont_l);
	UnloadFont(Phifont_m);
	UnloadFont(Phifont_s);
	CloseAudioDevice();
	CloseWindow();

	return 0;
}