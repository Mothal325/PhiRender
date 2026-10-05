#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <vector>
#include "offchart.h"

#include "easeType.h"
#include "Constants.h"

using namespace OFF;
using json = nlohmann::json;

static void Readnote(const json& data, std::vector<Note>& note)
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

static void Readevent(const json& data, std::vector<Event>& event, int mode)	//mode: speed 0, move 1, another 2
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

static void ReadBlockArea(const json& data, std::vector<BlockArea>& block)
{
	for (int i = 0; i < data.size(); i++)
	{
		block[i].x1 = data[i]["topRightPercentage"]["x"];
		block[i].y1 = data[i]["topRightPercentage"]["y"];
		block[i].x2 = data[i]["bottomLeftPercentage"]["x"];
		block[i].y2 = data[i]["bottomLeftPercentage"]["y"];
		block[i].appearTime = data[i]["appearTime"];
		block[i].enableTime = data[i]["enableTime"];
		block[i].disableTime = data[i]["disableTime"];
		block[i].disappearTime = data[i]["disappearTime"];
		block[i].isSubtract = data[i]["isSubtract"];
		block[i].rotateEvents.resize(data[i]["rotateEvents"].size());
		for (int j = 0; j < data[i]["rotateEvents"].size(); j++)
		{
			block[i].rotateEvents[j].x1 = data[i]["rotateEvents"][j]["anchor"]["x"];
			block[i].rotateEvents[j].y1 = data[i]["rotateEvents"][j]["anchor"]["y"];
			block[i].rotateEvents[j].time = data[i]["rotateEvents"][j]["time"];
			block[i].rotateEvents[j].easeType1 = data[i]["rotateEvents"][j]["easeType"];
			block[i].rotateEvents[j].value = data[i]["rotateEvents"][j]["rotation"];
		}
		block[i].moveEvents.resize(data[i]["moveEvents"].size());
		for (int j = 0; j < data[i]["moveEvents"].size(); j++)
		{
			block[i].moveEvents[j].x1 = data[i]["moveEvents"][j]["endPosition"]["x"];
			block[i].moveEvents[j].y1 = data[i]["moveEvents"][j]["endPosition"]["y"];
			block[i].moveEvents[j].time = data[i]["moveEvents"][j]["time"];
			block[i].moveEvents[j].easeType1 = data[i]["moveEvents"][j]["easeTypeX"];
			block[i].moveEvents[j].easeType2 = data[i]["moveEvents"][j]["easeTypeY"];
		}
		block[i].scaleEvents.resize(data[i]["scaleEvents"].size());
		for (int j = 0; j < data[i]["scaleEvents"].size(); j++)
		{
			block[i].scaleEvents[j].x1 = data[i]["scaleEvents"][j]["anchor"]["x"];
			block[i].scaleEvents[j].y1 = data[i]["scaleEvents"][j]["anchor"]["y"];
			block[i].scaleEvents[j].x2 = data[i]["scaleEvents"][j]["scale"]["x"];
			block[i].scaleEvents[j].y2 = data[i]["scaleEvents"][j]["scale"]["y"];
			block[i].scaleEvents[j].time = data[i]["scaleEvents"][j]["time"];
			block[i].scaleEvents[j].easeType1 = data[i]["scaleEvents"][j]["easeTypeX"];
			block[i].scaleEvents[j].easeType2 = data[i]["scaleEvents"][j]["easeTypeY"];
		}
	}
}

static void CalculateFloor(judgeLine& line)
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
	if (data.contains("blockAreaList"))
	{
		std::cout << "event " << event_sum << " note " << note_sum << "\n";
		//blockAreaList
		std::cout << "blockAreaList " << data["blockAreaList"].size() << "\n";
		blockAreaList.resize(data["blockAreaList"].size());
		ReadBlockArea(data["blockAreaList"], blockAreaList);
	}
}

void OFF::Linedata::FindLine(const judgeLine& line, float time)
{
	float t = time * line.bpm / OFF_T;
	//x and y
	for (int i = index[0]; i < line.moveEvents.size(); i++)
	{
		if (t < line.moveEvents[i].endTime)
		{
			x = (line.moveEvents[i].end - line.moveEvents[i].start) * (t - line.moveEvents[i].startTime) / (line.moveEvents[i].endTime - line.moveEvents[i].startTime) + line.moveEvents[i].start;
			y = (line.moveEvents[i].end2 - line.moveEvents[i].start2) * (t - line.moveEvents[i].startTime) / (line.moveEvents[i].endTime - line.moveEvents[i].startTime) + line.moveEvents[i].start2;
			index[0] = i;
			break;
		}
	}
	//rotate
	for (int i = index[1]; i < line.rotateEvents.size(); i++)
	{
		if (t < line.rotateEvents[i].endTime)
		{
			r = (line.rotateEvents[i].end - line.rotateEvents[i].start) * (t - line.rotateEvents[i].startTime) / (line.rotateEvents[i].endTime - line.rotateEvents[i].startTime) + line.rotateEvents[i].start;
			index[1] = i;
			break;
		}
	}
	//alpha
	for (int i = index[2]; i < line.disappearEvents.size(); i++)
	{
		if (t < line.disappearEvents[i].endTime)
		{
			a = (line.disappearEvents[i].end - line.disappearEvents[i].start) * (t - line.disappearEvents[i].startTime) / (line.disappearEvents[i].endTime - line.disappearEvents[i].startTime) + line.disappearEvents[i].start;
			index[2] = i;
			break;
		}
	}
	//floor and speed
	for (int i = index[3]; i < line.floorEvents.size(); i++)
	{
		if (t < line.floorEvents[i].endTime)
		{
			f = (line.floorEvents[i].end - line.floorEvents[i].start) * (t - line.floorEvents[i].startTime) / (line.floorEvents[i].endTime - line.floorEvents[i].startTime) + line.floorEvents[i].start;
			s = line.speedEvents[i].start;
			index[3] = i;
			break;
		}
	}
	//bpm
	bpm = line.bpm;
}

std::vector<OFF::Notedata> OFF::ReadNotedata(const OFF::Chartdata& data)
{
	std::vector<OFF::Notedata> notedata;
	std::unordered_map<int, int> hitCount;
	for (int i = 0; i < data.lines.size(); i++)
	{
		OFF::judgeLine aline = data.lines[i];
		for (int j = 0; j < aline.notesAbove.size(); j++)
		{
			OFF::Note anote = aline.notesAbove[j];
			OFF::Notedata noted = {};
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
			OFF::Notedata noted = {};
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

void OFF::Blockdata::FindBlock(const BlockArea& block, float time)
{
	if (time < block.appearTime || time > block.disappearTime)
	{
		state = 0;
		return;
	}
	else if (time < block.enableTime)
	{
		state = 1;
	}
	else if (time < block.disableTime)
	{
		state = 2;
	}
	else
	{
		state = 3;
	}
	if (block.isSubtract)
	{
		state += 4;
	}

	float blockcx = (block.x1 + block.x2) * 0.5f;
	float blockcy = (block.y1 + block.y2) * 0.5f;
	float blockx = blockcx * SW;
	float blocky = blockcy * SH;
	float blockw = std::abs(block.x1 - block.x2);
	float blockh = std::abs(block.y1 - block.y2);
	
	float sx = 1.0f, sy = 1.0f;
	float lastsx = block.scaleEvents.empty() ? 1.0f : block.scaleEvents[0].x2;
	float lastsy = block.scaleEvents.empty() ? 1.0f : block.scaleEvents[0].y2;
	for (int i = 0; i < block.scaleEvents.size(); i++)
	{
		if (time < block.scaleEvents[i].time)
		{
			break;
		}
		float blocksx = 1.0f, blocksy = 1.0f;
		float blockasx = blockcx, blockasy = blockcy;
		if (time >= block.scaleEvents[i].time)
		{
			if (i == block.scaleEvents.size() - 1)
			{
				sx = block.scaleEvents[i].x2;
				sy = block.scaleEvents[i].y2;
				blocksx = block.scaleEvents[i].x2;
				blocksy = block.scaleEvents[i].y2;
			}
			else if (time >= block.scaleEvents[i + 1].time)
			{
				blocksx = block.scaleEvents[i + 1].x2;
				blocksy = block.scaleEvents[i + 1].y2;
			}
			else
			{
				float p = (time - block.scaleEvents[i].time) / (block.scaleEvents[i + 1].time - block.scaleEvents[i].time);
				float xp = EaseTable(block.scaleEvents[i].easeType1, p);
				float yp = EaseTable(block.scaleEvents[i].easeType2, p);
				blocksx = block.scaleEvents[i].x2 * (1 - xp) + block.scaleEvents[i + 1].x2 * xp;
				blocksy = block.scaleEvents[i].y2 * (1 - yp) + block.scaleEvents[i + 1].y2 * yp;
				sx = block.scaleEvents[i].x2 * (1 - xp) + block.scaleEvents[i + 1].x2 * xp;
				sy = block.scaleEvents[i].y2 * (1 - yp) + block.scaleEvents[i + 1].y2 * yp;
			}
		}
		blockasx = block.scaleEvents[i].x1;
		blockasy = block.scaleEvents[i].y1;

		blockx += (blockx - blockasx * SW) * (std::abs(lastsx) > 1e-6 ? (blocksx - lastsx) / lastsx : 1.0f);
		blocky += (blocky - blockasy * SH) * (std::abs(lastsy) > 1e-6 ? (blocksy - lastsy) / lastsy : 1.0f);
		
		lastsx = blocksx;
		lastsy = blocksy;
	}
	
	float r = 0.0f;
	float lastr = block.rotateEvents.empty() ? 0.0f : block.rotateEvents[0].value;
	for (int i = 0; i < block.rotateEvents.size(); i++)
	{
		if (time < block.rotateEvents[i].time)
		{
			break;
		}
		float blockr = 0.0f;
		float blockarx = blockcx, blockary = blockcy;
		if (time >= block.rotateEvents[i].time)
		{
			if (i == block.rotateEvents.size() - 1)
			{
				r = block.rotateEvents[i].value;
				blockr = block.rotateEvents[i].value;
			}
			else if (time >= block.rotateEvents[i + 1].time)
			{
				blockr = block.rotateEvents[i + 1].value;
			}
			else
			{
				float p = (time - block.rotateEvents[i].time) / (block.rotateEvents[i + 1].time - block.rotateEvents[i].time);
				float rp = EaseTable(block.rotateEvents[i].easeType1, p);
				blockr = block.rotateEvents[i].value * (1 - rp) + block.rotateEvents[i + 1].value * rp;
				r = block.rotateEvents[i].value * (1 - rp) + block.rotateEvents[i + 1].value * rp;
			}
		}
		blockarx = block.rotateEvents[i].x1;
		blockary = block.rotateEvents[i].y1;

		float theta = (blockr - lastr) / 180.0f * EASE_PI;
		float ct = cos(theta), st = sin(theta);
		float rdx = blockx - blockarx * SW;
		float rdy = blocky - blockary * SH;
		blockx += rdx * ct - rdy * st - rdx;
		blocky += rdx * st + rdy * ct - rdy;
		lastr = blockr;
	}

	float blockmx = blockcx, blockmy = blockcy;
	for (int i = 0; i < block.moveEvents.size(); i++)
	{
		if (time >= block.moveEvents[i].time)
		{
			if (i == block.moveEvents.size() - 1)
			{
				blockmx = block.moveEvents[i].x1;
				blockmy = block.moveEvents[i].y1;
			}
			else if (time >= block.moveEvents[i + 1].time)
			{
				continue;
			}
			else
			{
				float p = (time - block.moveEvents[i].time) / (block.moveEvents[i + 1].time - block.moveEvents[i].time);
				float xp = EaseTable(block.moveEvents[i].easeType1, p);
				float yp = EaseTable(block.moveEvents[i].easeType2, p);
				blockmx = block.moveEvents[i].x1 * (1 - xp) + block.moveEvents[i + 1].x1 * xp;
				blockmy = block.moveEvents[i].y1 * (1 - yp) + block.moveEvents[i + 1].y1 * yp;
			}
		}
	}

	float mdx = blockmx - blockcx;
	float mdy = blockmy - blockcy;
	blockx += mdx * SW;
	blocky += mdy * SH;
	
	x = blockx;
	y = blocky;
	w = blockw * std::abs(sx) * SW;
	h = blockh * std::abs(sy) * SH;
	rotation = r;
}