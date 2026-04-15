#pragma once
#include <vector>
#include <string>

#define OFF_X (1.0 / 18.0)
#define OFF_Y 0.6
#define OFF_T 1.875

namespace OFF
{
	struct Note
	{
		int type;
		int time;
		float positionX;
		int holdTime;
		float speed;
		float floorPosition;
	};

	struct Event	//speed -> start, move -> all, another -> start end
	{
		int startTime;
		int endTime;
		float start;
		float end;
		float start2;
		float end2;
	};

	struct judgeLine
	{
		float bpm;
		std::vector<Note> notesAbove;
		std::vector<Note> notesBelow;
		std::vector<Event> speedEvents;
		std::vector<Event> floorEvents;		//from speed
		std::vector<Event> moveEvents;
		std::vector<Event> rotateEvents;
		std::vector<Event> disappearEvents;
	};

	struct Chartdata
	{
		int formatVersion;
		float offset;
		std::vector<judgeLine> lines;
	};

	struct Linedata
	{
		float x;	//xPosition
		float y;	//yPosition
		float r;	//rotation
		float a;	//alpha
		float f;	//floor
		float s;	//speed
		float bpm;
		int index[4] = { 0 };
	};

	struct Notedata
	{
		Note note;
		int lineid;
		bool isAbove;
		bool ismh;
		bool isPlayed;
	};

	Chartdata Readdata(std::string filename);

	void FindLine(judgeLine line, float time, Linedata& data);
}