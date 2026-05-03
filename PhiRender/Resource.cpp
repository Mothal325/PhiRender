#include <raylib.h>
#include "Resource.h"

void Resource::LoadResource(void)
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
void Resource::UnloadResource(void)
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
