#include <format>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "include\raylib.h"
#include "Constants.h"
#include "offchart.h"
#include "NoteTexture.h"
#include "State.h"
#include "HitEffect.h"
#include "HitSound.h"
#include "MixMusic.h"

int main(void)
{
	std::string chartname, songname, backgroundname, videoname;
	std::cout << "谱面文件路径：";
	std::getline(std::cin, chartname);
	
	OFF::Chartdata data;
	data.Readdata(chartname);

	std::cout << "音频文件路径：";
	std::getline(std::cin, songname);
	InitAudioDevice();
	Music bgm = LoadMusicStream(songname.c_str());

	std::cout << "背景文件路径：";
	std::getline(std::cin, backgroundname);

	std::cout << "渲染视频位置（留空代表不渲染视频）：";
	std::getline(std::cin, videoname);
	bool isrender = true;
	FILE* ffmpegpipe = nullptr;
	if (videoname.empty())
	{
		isrender = false;
	}
	else
	{
		std::cout << "混音中……\n";
		MixMusic(data, songname);
		std::cout << "渲染中……\n";
		std::string cmd = std::format(
			"ffmpeg -y -loglevel error "
			"-f rawvideo -pix_fmt rgba -s {}x{} -r {} -i - "
			"-i temp.m4a "
			"-map 0:v -map 1:a "
			"-c:v libx264 -preset fast -pix_fmt yuv420p "
			"-c:a copy "
			"-shortest {}",
			SW, SH, RFPS, videoname);
		ffmpegpipe = _popen(cmd.c_str(), "wb");
	}

	SetTraceLogLevel(LOG_NONE);
	SetExitKey(KEY_NULL);
	InitWindow(SW, SH, "Mothal's Phigros Render");

	Image bg_image = LoadImage(backgroundname.c_str());
	ImageBlurGaussian(&bg_image, BLURSIZE);
	Texture bg = LoadTextureFromImage(bg_image);
	UnloadImage(bg_image);
	
	HitEffectManager EffectM;

	HitSoundManager SoundM;
	SoundM.Init(data);
	SoundM.SetHitSoundVolume(isrender ? 0.0f : 0.5f);

	State GameState;
	GameState.Init(data);

	NoteTexture res;
	res.LoadResource();

	Font Phifont_s = LoadFontEx("res/Phifont.ttf", (int)(SH * 0.025), NULL, 0);
	Font Phifont_m = LoadFontEx("res/Phifont.ttf", (int)(SH * 0.05), NULL, 0);
	Font Phifont_l = LoadFontEx("res/Phifont.ttf", (int)(SH * 0.075), NULL, 0);

	SetMusicVolume(bgm, 0.5f);
	float bgmspeed = BGMSPEED;
	SetMusicPitch(bgm, bgmspeed);
	SetTextureFilter(bg, TEXTURE_FILTER_TRILINEAR);
	
	SetTargetFPS(FPS);
	bool isPlaying = false;
	bool shouldPlaying = false;
	char timetext[64] = "";
	char combotext[64] = "";
	char scoretext[64] = "";
	float playtime = 0.0f;

	while (!WindowShouldClose())
	{
		if (!isrender)
		{
			playtime = GetMusicTimePlayed(bgm);
			if (GetMusicTimeLength(bgm) - playtime < 0.1f || IsKeyPressed(KEY_R))
			{
				StopMusicStream(bgm);
				playtime = 0.0f;
				isPlaying = false;
				shouldPlaying = false;
				GameState.Reset();
				EffectM.Clear();
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

			if (IsKeyPressed(KEY_UP))
			{
				bgmspeed += 0.1f;
			}
			if (IsKeyPressed(KEY_DOWN))
			{
				bgmspeed -= 0.1f;
			}
			if (IsKeyPressed(KEY_RIGHT) && playtime < GetMusicTimeLength(bgm) - 2.0f)
			{
				SeekMusicStream(bgm, playtime + 1.0f);
				playtime += 1.0f;
			}
			if (IsKeyPressed(KEY_LEFT) && playtime > 2.0f)
			{
				GameState.TimeJump(playtime - 1.0f, data.lines, EffectM);
				SeekMusicStream(bgm, playtime - 1.0f);
				playtime -= 1.0f;
			}
			SetMusicPitch(bgm, bgmspeed);
			UpdateMusicStream(bgm);
		}
		
		GameState.Update(playtime, data.lines, EffectM, SoundM);
		GameState.UpdateBlock(playtime, data.blockAreaList);

		BeginDrawing();

		ClearBackground(BLACK);
		
		DrawTexturePro(bg, { 0, 0, (float)bg.width, (float)bg.height }, { (SW - (float)SH / bg.height * bg.width) / 2, 0, (float)SH / bg.height * bg.width, SH }, { 0, 0 }, 0, {BRIGHTNESS, BRIGHTNESS, BRIGHTNESS, 255});

		GameState.Draw(playtime, res);
		EffectM.DrawHitEffect(playtime);

		int score = (int)((double)GameState.GetHitNum() / GameState.GetNoteNum() * 1000000.0 + 0.5);

		std::snprintf(timetext, sizeof(timetext), "Time:%.1f/%.1f(%.1fx)\nFPS:%d", GetMusicTimePlayed(bgm), GetMusicTimeLength(bgm), bgmspeed, GetFPS());
		std::snprintf(combotext, sizeof(combotext), "%d", GameState.GetHitNum());
		std::snprintf(scoretext, sizeof(scoretext), "%07d", score);
		if (!isrender)
		{
			DrawTextEx(Phifont_s, timetext, { 0.0f, 0.0f }, SH * 0.025f, 0.0f, WHITE);
		}
		DrawTextEx(Phifont_l, combotext, { (SW - MeasureTextEx(Phifont_l, combotext, SH * 0.075f, 0).x) / 2.0f, SH * 0.015f }, SH * 0.075f, 0.0f, WHITE);
		DrawTextEx(Phifont_s, "COMBO", { (SW - MeasureTextEx(Phifont_s, "COMBO", SH * 0.025f, 0).x) / 2.0f, SH * 0.1f }, SH * 0.025f, 0.0f, WHITE);
		DrawTextEx(Phifont_m, scoretext, { (float)SW - MeasureTextEx(Phifont_m, scoretext, SH * 0.05f, 0.72f).x - SH * 0.04f, SH * 0.03f }, SH * 0.05f, 0.72f, WHITE);

		EndDrawing();

		if (isrender)
		{
			Image screenimg = LoadImageFromScreen();
			fwrite(screenimg.data, sizeof(Color), SW * SH, ffmpegpipe);
			UnloadImage(screenimg);
			if (GetMusicTimeLength(bgm) - playtime > 0.0f)
			{
				playtime += 1.0f / RFPS;
			}
			else
			{
				break;
			}
		}
	}
	
	if (ffmpegpipe)
	{
		_pclose(ffmpegpipe);
	}
	UnloadMusicStream(bgm);
	SoundM.Free();
	UnloadTexture(bg);
	res.UnloadResource();
	UnloadFont(Phifont_l);
	UnloadFont(Phifont_m);
	UnloadFont(Phifont_s);
	CloseAudioDevice();
	CloseWindow();
	
	return 0;
}