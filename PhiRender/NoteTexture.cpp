#include <raylib.h>
#include "Constants.h"
#include "NoteTexture.h"

void NoteTexture::LoadResource(void)
{
	Tclick = LoadTexture("res/click.png");
	Tclick_mh = LoadTexture("res/click_mh.png");
	Tdrag = LoadTexture("res/drag.png");
	Tdrag_mh = LoadTexture("res/drag_mh.png");
	Tflick = LoadTexture("res/flick.png");
	Tflick_mh = LoadTexture("res/flick_mh.png");
	Thold = LoadTexture("res/hold.png");
	Thold_mh = LoadTexture("res/hold_mh.png");
	holdAtlas[0] = 50, holdAtlas[1] = 50;
	holdAtlasMH[0] = 50, holdAtlasMH[1] = 50;
}
void NoteTexture::UnloadResource(void)
{
	UnloadTexture(Tclick);
	UnloadTexture(Tdrag);
	UnloadTexture(Tflick);
	UnloadTexture(Tclick_mh);
	UnloadTexture(Tdrag_mh);
	UnloadTexture(Tflick_mh);
	UnloadTexture(Thold);
	UnloadTexture(Thold_mh);
}

void NoteTexture::DrawNoteTexture(int type, bool ismh, float x, float y, float r)
{
	int mhnumber = ismh ? 4 : 0;
	switch (type + mhnumber)
	{
	case 1:
		DrawTexturePro(Tclick, { 0, 0, (float)Tclick.width, (float)Tclick.height }, { x, SH - y, NW, (float)Tclick.height / (float)Tclick.width * NW }, { NW / 2.0f, (float)Tclick.height / (float)Tclick.width * NW / 2.0f }, r, WHITE);
		break;
	case 2:
		DrawTexturePro(Tdrag, { 0, 0, (float)Tdrag.width, (float)Tdrag.height }, { x, SH - y, NW, (float)Tdrag.height / (float)Tdrag.width * NW }, { NW / 2.0f, (float)Tdrag.height / (float)Tdrag.width * NW / 2.0f }, r, WHITE);
		break;
	case 4:
		DrawTexturePro(Tflick, { 0, 0, (float)Tflick.width, (float)Tflick.height }, { x, SH - y, NW, (float)Tflick.height / (float)Tflick.width * NW }, { NW / 2.0f, (float)Tflick.height / (float)Tflick.width * NW / 2.0f }, r, WHITE);
		break;
	case (1 + 4):
		DrawTexturePro(Tclick_mh, { 0, 0, (float)Tclick_mh.width, (float)Tclick_mh.height }, { x, SH - y, (float)Tclick_mh.width / (float)Tclick.width * NW, (float)Tclick_mh.height / (float)Tclick.width * NW }, { (float)Tclick_mh.width / (float)Tclick.width * NW / 2.0f, (float)Tclick_mh.height / (float)Tclick.width * NW / 2.0f }, r, WHITE);
		break;
	case (2 + 4):
		DrawTexturePro(Tdrag_mh, { 0, 0, (float)Tdrag_mh.width, (float)Tdrag_mh.height }, { x, SH - y, (float)Tdrag_mh.width / (float)Tdrag.width * NW, (float)Tdrag_mh.height / (float)Tdrag.width * NW }, { (float)Tdrag_mh.width / (float)Tdrag.width * NW / 2.0f, (float)Tdrag_mh.height / (float)Tdrag.width * NW / 2.0f }, r, WHITE);
		break;
	case (4 + 4):
		DrawTexturePro(Tflick_mh, { 0, 0, (float)Tflick_mh.width, (float)Tflick_mh.height }, { x, SH - y, (float)Tflick_mh.width / (float)Tflick.width * NW, (float)Tflick_mh.height / (float)Tflick.width * NW }, { (float)Tflick_mh.width / (float)Tflick.width * NW / 2.0f, (float)Tflick_mh.height / (float)Tflick.width * NW / 2.0f }, r, WHITE);
		break;
	}
}

void NoteTexture::DrawHoldTexture(bool hit, bool ismh, float x, float y, float r, float length, int updown)
{
	if (ismh)
	{
		float holdmhwidth = (float)Thold_mh.width / (float)Thold.width * NW;
		DrawTexturePro(Thold_mh, { 0, (float)holdAtlasMH[0], (float)Thold_mh.width, (float)Thold_mh.height - holdAtlasMH[1] - holdAtlasMH[0] }, { x, SH - y, holdmhwidth, length }, { holdmhwidth / 2.0f, length }, r + 180.0 * updown, WHITE);
		DrawTexturePro(Thold_mh, { 0, 0, (float)Thold_mh.width, (float)holdAtlasMH[0] }, { x, SH - y, holdmhwidth, (float)holdAtlasMH[0] / (float)Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, length + (float)holdAtlasMH[0] / (float)Thold_mh.width * holdmhwidth }, r + 180.0 * updown, WHITE);
		if (!hit)
		{
			DrawTexturePro(Thold_mh, { 0, (float)Thold_mh.height - holdAtlasMH[1], (float)Thold_mh.width, (float)holdAtlasMH[1] }, { x, SH - y, holdmhwidth, (float)holdAtlasMH[1] / (float)Thold_mh.width * holdmhwidth }, { holdmhwidth / 2.0f, 0.0f }, r + 180.0 * updown, WHITE);
		}
	}
	else
	{
		DrawTexturePro(Thold, { 0, (float)holdAtlas[0], (float)Thold.width, (float)Thold.height - holdAtlas[1] - holdAtlas[0] }, { x, SH - y, NW, length }, { NW / 2.0f, length }, r + 180.0 * updown, WHITE);
		DrawTexturePro(Thold, { 0, 0, (float)Thold.width, (float)holdAtlas[0] }, { x, SH - y, NW, (float)holdAtlas[0] / (float)Thold.width * NW }, { NW / 2.0f, length + (float)holdAtlas[0] / (float)Thold.width * NW }, r + 180.0 * updown, WHITE);
		if (!hit)
		{
			DrawTexturePro(Thold, { 0, (float)Thold.height - holdAtlas[1], (float)Thold.width, (float)holdAtlas[1] }, { x, SH - y, NW, (float)holdAtlas[1] / (float)Thold.width * NW }, { NW / 2.0f, 0.0f }, r + 180.0 * updown, WHITE);

		}
	}
}