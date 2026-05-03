#pragma once
#include <raylib.h>

struct Resource
{
	void LoadResource(void);
	void UnloadResource(void);

	Texture Tclick, Tclick_mh, Tdrag, Tdrag_mh, Tflick, Tflick_mh, Thold, Thold_mh;
	int holdAtlas[2] = { 0 }, holdAtlasMH[2] = { 0 };
};