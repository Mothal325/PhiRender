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

	struct BlockEvent
	{
		float x1;
		float x2;
		float y1;
		float y2;
		float time;
		float value;
		int easeType1;
		int easeType2;
	};

	struct BlockArea
	{
		float x1;
		float x2;
		float y1;
		float y2;
		float appearTime;
		float enableTime;
		float disableTime;
		float disappearTime;
		bool isSubtract;
		std::vector<BlockEvent> rotateEvents;
		std::vector<BlockEvent> moveEvents;
		std::vector<BlockEvent> scaleEvents;
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
		std::vector<BlockArea> blockAreaList;

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

	struct Blockdata
	{
		float x;
		float y;
		float w;
		float h;
		float rotation;
		int state; //0 -> skip, 1 -> appear, 2 -> enable, 3 -> disable, +4 -> sub

		void FindBlock(const BlockArea& block, float time);
	};

	std::vector<Notedata> ReadNotedata(const Chartdata& data);
}