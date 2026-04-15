#pragma once
#include <vector>
#include <string>

namespace RPE
{
	struct Beat
	{
		int beat;
		int num;
		int den;
	};

	struct BPMdata
	{
		float bpm;
		Beat startTime;
	};

	struct META
	{
		int RPEVersion;
		int offset;
		std::string background;
		std::string charter;
		std::string composer;
		std::string id;
		std::string illustration;
		std::string level;
		std::string name;
		std::string song;
	};

	struct Event
	{
		int easingType;
		Beat startTime;
		Beat endTime;
		float start;
		float end;
		int linkgroup;
	};

	struct colorEvent
	{
		int easingType;
		Beat startTime;
		Beat endTime;
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
		Beat startTime;
		Beat endTime;
	};

	struct judgeLine
	{
		std::string Name;
		std::string Texture;
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

	Chartdata Readdata(std::string filename);

}