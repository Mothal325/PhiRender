#pragma once
#include <raylib.h>

struct NoteTexture
{
	void LoadResource(void);
	void UnloadResource(void);

	void DrawNoteTexture(int type, bool ismh, float x, float y, float r);
	void DrawHoldTexture(bool hit, bool ismh, float x, float y, float r, float length, int updown);

private:
	Texture Tclick, Tclick_mh, Tdrag, Tdrag_mh, Tflick, Tflick_mh, Thold, Thold_mh;
	int holdAtlas[2] = { 0 }, holdAtlasMH[2] = { 0 };
};