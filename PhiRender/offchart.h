#pragma once
#include <vector>
#include <string>

constexpr float OFF_X = 1.0f / 18.0f;
constexpr float OFF_Y = 0.6f;
constexpr float OFF_T = 1.875f;
//time = t * OFF_T / bpm
//t = time * bpm / OFF_T

namespace OFF
{
	struct Note
	{
		int type;	//1 -> Tap, 2 -> Drag, 3 -> Hold, 4 -> Flick
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

		void Readdata(std::string filename);
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

		void FindLine(const judgeLine& line, float time);
	};

	struct Notedata
	{
		Note note;
		int lineid;
		bool isAbove;
		bool ismh;
		bool isPlayed;
	};

	std::vector<Notedata> ReadNotedata(const Chartdata& data);
}