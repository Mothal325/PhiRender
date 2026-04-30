#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <vector>
#include "offchart.h"

using namespace OFF;
using json = nlohmann::json;

void Readnote(const json& data, std::vector<Note>& note)
{
	for (int i = 0; i < data.size(); i++)
	{
		note[i].type = data[i]["type"];
		note[i].time = data[i]["time"];
		note[i].positionX = data[i]["positionX"];
		note[i].holdTime = data[i]["holdTime"];
		note[i].speed = data[i]["speed"];
		note[i].floorPosition = data[i]["floorPosition"];
	}
}

void Readevent(const json& data, std::vector<Event>& event, int mode)	//mode: speed 0, move 1, another 2
{
	for (int i = 0; i < data.size(); i++)
	{
		event[i].startTime = data[i]["startTime"];
		event[i].endTime = data[i]["endTime"];
		if (mode == 0)
		{
			event[i].start = data[i]["value"];
		}
		else
		{
			event[i].start = data[i]["start"];
			event[i].end = data[i]["end"];
		}
		if (mode == 1)
		{
			event[i].start2 = data[i]["start2"];
			event[i].end2 = data[i]["end2"];
		}
	}
}

void CalculateFloor(judgeLine& line)
{
	float s = 0.0, t = 0.0;
	for (int i = 0; i < line.speedEvents.size(); i++)
	{
		line.floorEvents[i].startTime = line.speedEvents[i].startTime;
		line.floorEvents[i].endTime = line.speedEvents[i].endTime;
		line.floorEvents[i].start = s;
		t = (line.speedEvents[i].endTime - line.speedEvents[i].startTime) * OFF_T / line.bpm;
		s += line.speedEvents[i].start * t;
		line.floorEvents[i].end = s;
	}
}

void OFF::Chartdata::Readdata(std::string filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "文件打不开喵\n";
		exit(-1);
	}
	json data;
	try
	{
		file >> data;
	}
	catch (const json::parse_error& e)
	{
		std::cerr << "JSON 解析错误喵：" << e.what() << std::endl;
		exit(-1);
	}
	int event_sum = 0, note_sum = 0;
	//基础信息
	formatVersion = data["formatVersion"];
	offset = data["offset"];
	//judgeLineList
	std::cout << "正在读取judgeLineList\n";
	lines.resize(data["judgeLineList"].size());
	for (int i = 0; i < data["judgeLineList"].size(); i++)
	{
		json jl = data["judgeLineList"][i];
		std::cout << i << "\n";
		lines[i].bpm = jl["bpm"];
		//notesAbove
		std::cout << "\tnotesAbove " << jl["notesAbove"].size() << "\n";
		note_sum += jl["notesAbove"].size();
		lines[i].notesAbove.resize(jl["notesAbove"].size());
		Readnote(jl["notesAbove"], lines[i].notesAbove);
		//notesBelow
		std::cout << "\tnotesBelow " << jl["notesBelow"].size() << "\n";
		note_sum += jl["notesBelow"].size();
		lines[i].notesBelow.resize(jl["notesBelow"].size());
		Readnote(jl["notesBelow"], lines[i].notesBelow);
		//speedEvent
		std::cout << "\tspeedEvent " << jl["speedEvents"].size() << "\n";
		event_sum += jl["speedEvents"].size();
		lines[i].speedEvents.resize(jl["speedEvents"].size());
		Readevent(jl["speedEvents"], lines[i].speedEvents, 0);
		lines[i].floorEvents.resize(jl["speedEvents"].size());
		CalculateFloor(lines[i]);
		//moveEvents
		std::cout << "\tmoveEvents " << jl["judgeLineMoveEvents"].size() << "\n";
		event_sum += jl["judgeLineMoveEvents"].size();
		lines[i].moveEvents.resize(jl["judgeLineMoveEvents"].size());
		Readevent(jl["judgeLineMoveEvents"], lines[i].moveEvents, 1);
		//rotateEvents
		std::cout << "\trotateEvents " << jl["judgeLineRotateEvents"].size() << "\n";
		event_sum += jl["judgeLineRotateEvents"].size();
		lines[i].rotateEvents.resize(jl["judgeLineRotateEvents"].size());
		Readevent(jl["judgeLineRotateEvents"], lines[i].rotateEvents, 2);
		//disappearEvents
		std::cout << "\tdisappearEvents " << jl["judgeLineDisappearEvents"].size() << "\n";
		event_sum += jl["judgeLineDisappearEvents"].size();
		lines[i].disappearEvents.resize(jl["judgeLineDisappearEvents"].size());
		Readevent(jl["judgeLineDisappearEvents"], lines[i].disappearEvents, 2);
	}
	std::cout << "event " << event_sum << " note " << note_sum << "\n";
}

void OFF::FindLine(const judgeLine &line, float time, Linedata& data)
{
	float t = time * line.bpm / OFF_T;
	//x and y
	for (int i = data.index[0]; i < line.moveEvents.size(); i++)
	{
		if (t < line.moveEvents[i].endTime)
		{
			data.x = (line.moveEvents[i].end - line.moveEvents[i].start) * (t - line.moveEvents[i].startTime) / (line.moveEvents[i].endTime - line.moveEvents[i].startTime) + line.moveEvents[i].start;
			data.y = (line.moveEvents[i].end2 - line.moveEvents[i].start2) * (t - line.moveEvents[i].startTime) / (line.moveEvents[i].endTime - line.moveEvents[i].startTime) + line.moveEvents[i].start2;
			data.index[0] = i;
			break;
		}
	}
	//rotate
	for (int i = data.index[1]; i < line.rotateEvents.size(); i++)
	{
		if (t < line.rotateEvents[i].endTime)
		{
			data.r = (line.rotateEvents[i].end - line.rotateEvents[i].start) * (t - line.rotateEvents[i].startTime) / (line.rotateEvents[i].endTime - line.rotateEvents[i].startTime) + line.rotateEvents[i].start;
			data.index[1] = i;
			break;
		}
	}
	//alpha
	for (int i = data.index[2]; i < line.disappearEvents.size(); i++)
	{
		if (t < line.disappearEvents[i].endTime)
		{
			data.a = (line.disappearEvents[i].end - line.disappearEvents[i].start) * (t - line.disappearEvents[i].startTime) / (line.disappearEvents[i].endTime - line.disappearEvents[i].startTime) + line.disappearEvents[i].start;
			data.index[2] = i;
			break;
		}
	}
	//floor and speed
	for (int i = data.index[3]; i < line.floorEvents.size(); i++)
	{
		if (t < line.floorEvents[i].endTime)
		{
			data.f = (line.floorEvents[i].end - line.floorEvents[i].start) * (t - line.floorEvents[i].startTime) / (line.floorEvents[i].endTime - line.floorEvents[i].startTime) + line.floorEvents[i].start;
			data.s = line.speedEvents[i].start;
			data.index[3] = i;
			break;
		}
	}
	//bpm
	data.bpm = line.bpm;
}

std::vector<OFF::Notedata> OFF::ReadNotedata(OFF::Chartdata data)
{
	std::vector<OFF::Notedata> notedata;
	std::unordered_map<int, int> hitCount;
	for (int i = 0; i < data.lines.size(); i++)
	{
		OFF::judgeLine aline = data.lines[i];
		for (int j = 0; j < aline.notesAbove.size(); j++)
		{
			OFF::Note anote = aline.notesAbove[j];
			OFF::Notedata noted;
			noted.note = anote;
			noted.lineid = i;
			noted.isAbove = true;
			noted.ismh = false;
			hitCount[anote.time]++;
			notedata.push_back(noted);
		}
		for (int j = 0; j < aline.notesBelow.size(); j++)
		{
			OFF::Note anote = aline.notesBelow[j];
			OFF::Notedata noted;
			noted.note = anote;
			noted.lineid = i;
			noted.isAbove = false;
			hitCount[anote.time]++;
			notedata.push_back(noted);
		}
	}
	for (int i = 0; i < notedata.size(); i++)
	{
		OFF::Note anote = notedata[i].note;
		if (hitCount[anote.time] > 1)
		{
			notedata[i].ismh = true;
		}
	}

	return notedata;
}