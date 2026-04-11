#pragma once

namespace RPE
{
	struct BPMdata
	{
		float bpm;
		int startTime[3];
	};

	struct META
	{
		int RPEVersion;
		char* background;
		char* charter;
		char* composer;
		char* id;
		char* illustration;
		char* level;
		char* name;
		int offset;
		char* song;
	};

	struct Event
	{
		int easingType;
		int startTime[3];
		int endTime[3];
		float start;
		float end;
		int linkgroup;
	};

	struct colorEvent
	{
		int easingType;
		int startTime[3];
		int endTime[3];
		unsigned char start[3];
		unsigned char end[3];
		int linkgroup;
	};

	struct eventLayer
	{
		std::vector<Event> alphaEvents;
		std::vector<Event> moveXEvents;
		std::vector<Event> moveYEvents;
		std::vector<Event> rotateEvents;
		std::vector<Event> speedEvents;
	};

	struct extended
	{
		std::vector<colorEvent> colorEvents;
		std::vector<Event> scaleXEvents;
		std::vector<Event> scaleYEvents;
	};

	struct Note
	{
		int above;
		int alpha;
		int isFake;
		float size;
		float speed;
		float visibleTime;
		float yOffset;
		int type;
		float positionX;
		int startTime[3];
		int endTime[3];
	};

	struct judgeLine
	{
		char* Name;
		char* Texture;
		float bpmfactor;
		int isCover;
		std::vector<eventLayer> eventLayers;
		std::vector<extended> extended;
		std::vector<Note> notes;
	};

	struct Chartdata
	{
		std::vector<BPMdata> bpm;
		META meta;
		std::vector<judgeLine> lines;
	};

	Chartdata Readdata(const char* filename);

	void Printdata(Chartdata data);	//bad

	void AddLine(Chartdata* data);

	void AddNote(Chartdata* data, int type, int line, int starttime[], int endtime[], int above, float speed, float positionX);	//above = 1 -> up, when down -> hold = 2, other = 0

	void AddEvent(Chartdata* data, char* eventtype, int line, int starttime[], int endtime[], int start[], int end[], int easingtype);
}