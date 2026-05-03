#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <raylib.h>
#include "Constants.h"
#include "offchart.h"
#include "HitEffect.h"

Texture Tclick, Tclick_mh, Tdrag, Tdrag_mh, Tflick, Tflick_mh, Thold, Thold_mh;
Sound Sclick, Sdrag, Sflick;
std::vector<Sound> Snotes;
int holdAtlas[2] = { 50, 50 }, holdAtlasMH[2] = { 50, 50 };

void InitNoteSound(const std::vector<OFF::Notedata> &notedata)
{
	for (int i = 0; i < notedata.size(); i++)
	{
		switch (notedata[i].note.type)
		{
		case 1:
		case 3: Snotes.push_back(LoadSoundAlias(Sclick)); break;
		case 2: Snotes.push_back(LoadSoundAlias(Sdrag));  break;
		case 4: Snotes.push_back(LoadSoundAlias(Sflick)); break;
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

int DrawNote(std::vector<OFF::Notedata> &notedata, const std::vector<OFF::Linedata> &data, float time, bool renderhold)
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
				case (1 + 4):
					DrawTexturePro(Tclick_mh, { 0, 0, (float)Tclick_mh.width, (float)Tclick_mh.height }, { x, SH - y, (float)Tclick_mh.width / (float)Tclick.width * NW, (float)Tclick_mh.height / (float)Tclick.width * NW }, { (float)Tclick_mh.width / (float)Tclick.width * NW / 2.0f, (float)Tclick_mh.height / (float)Tclick.width * NW / 2.0f }, rotation, WHITE);
					break;
				case (2 + 4):
					DrawTexturePro(Tdrag_mh, { 0, 0, (float)Tdrag_mh.width, (float)Tdrag_mh.height }, { x, SH - y, (float)Tdrag_mh.width / (float)Tdrag.width * NW, (float)Tdrag_mh.height / (float)Tdrag.width * NW }, { (float)Tdrag_mh.width / (float)Tdrag.width * NW / 2.0f, (float)Tdrag_mh.height / (float)Tdrag.width * NW / 2.0f }, rotation, WHITE);
					break;
				case (4 + 4):
					DrawTexturePro(Tflick_mh, { 0, 0, (float)Tflick_mh.width, (float)Tflick_mh.height }, { x, SH - y, (float)Tflick_mh.width / (float)Tflick.width * NW, (float)Tflick_mh.height / (float)Tflick.width * NW }, { (float)Tflick_mh.width / (float)Tflick.width * NW / 2.0f, (float)Tflick_mh.height / (float)Tflick.width * NW / 2.0f }, rotation, WHITE);
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

void DrawJudgeLine(const OFF::judgeLine &line, float time, OFF::Linedata &data)
{
	data.FindLine(line, time);
	Color c = { 255, 255, 255, data.a * 255 };
	Rectangle l = { data.x * SW, (1.0 - data.y) * SH, LINEL * SH, LINEW * SH };
	DrawRectanglePro(l, { l.width / 2, l.height / 2 }, -data.r, c);
}

int main(void)
{
	std::string chartname, songname, backgroundname;
	std::cout << "谱面文件路径：";
	std::getline(std::cin, chartname);
	
	OFF::Chartdata data;
	data.Readdata(chartname);
	std::vector<OFF::Notedata> notedata = OFF::ReadNotedata(data);

	std::cout << "音频文件路径：";
	std::getline(std::cin, songname);
	InitAudioDevice();
	Music bgm = LoadMusicStream(songname.c_str());

	std::cout << "背景文件路径：";
	std::getline(std::cin, backgroundname);

	SetTraceLogLevel(LOG_NONE);
	InitWindow(SW, SH, "Mothal's Phigros Render");
	Image bg_image = LoadImage(backgroundname.c_str());
	ImageBlurGaussian(&bg_image, BLURSIZE);
	Texture bg = LoadTextureFromImage(bg_image);
	
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
	SetMusicVolume(bgm, 0.5f);
	SetMusicPitch(bgm, BGMSPEED);
	SetTextureFilter(bg, TEXTURE_FILTER_TRILINEAR);
	
	SetTargetFPS(FPS);
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

	HitEffectManager Effect;

	while (!WindowShouldClose())
	{
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
			Effect.clear();
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
		
		DrawTexturePro(bg, { 0, 0, (float)bg.width, (float)bg.height }, { (SW - (float)SH / bg.height * bg.width) / 2, 0, (float)SH / bg.height * bg.width, SH }, { 0, 0 }, 0, {BRIGHTNESS, BRIGHTNESS, BRIGHTNESS, 255});
		float playtime = GetMusicTimePlayed(bgm);
		for (int i = 0; i < data.lines.size(); i++)
		{
			DrawJudgeLine(data.lines[i], playtime, linedata[i]);
		}

		hitnum = DrawNote(notedata, linedata, playtime, true);
		hitnum = DrawNote(notedata, linedata, playtime, false);

		Effect.AddEffect(linedata, notedata, playtime);
		Effect.UpdateHoldHitEffect(linedata, playtime);
		Effect.DrawHitEffect(playtime);

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