#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "rpechart.h"

using namespace RPE;
using json = nlohmann::json;

void Readtime(const json& data, Beat& time)
{
	time.beat = data[0];
	time.num = data[1];
	time.den = data[2];
}

void Readnote(const json& data, std::vector<Note>& note)
{
	for (int i = 0; i < data.size(); i++)
	{
		note[i].above = data[i]["above"];
		note[i].alpha = data[i]["alpha"];
		Readtime(data[i]["endTime"], note[i].endTime);
		note[i].isFake = data[i]["isFake"];
		note[i].positionX = data[i]["positionX"];
		note[i].size = data[i]["size"];
		note[i].speed = data[i]["speed"];
		Readtime(data[i]["startTime"], note[i].startTime);
		note[i].type = data[i]["type"];
		note[i].visibleTime = data[i]["visibleTime"];
		note[i].yOffset = data[i]["yOffset"];
	}
}

void Readevent(const json& data, std::vector<Event>& event)
{
	for (int i = 0; i < data.size(); i++)
	{
		event[i].easingType = data[i].value("easingType", 1);
		event[i].start = data[i]["start"];
		event[i].end = data[i]["end"];
		Readtime(data[i]["startTime"], event[i].startTime);
		Readtime(data[i]["endTime"], event[i].endTime);
		event[i].linkgroup = data[i]["linkgroup"];
	}
}

Chartdata RPE::Readdata(std::string filename)
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
	//解析
	Chartdata chartdata;
	int event_sum = 0, note_sum = 0;
	//BPM
	std::cout << "正在读取BPM\n";
	chartdata.bpm.resize(data["BPMList"].size());
	for (int i = 0; i < data["BPMList"].size(); i++)
	{
		chartdata.bpm[i].bpm = data["BPMList"][i]["bpm"];
		Readtime(data["BPMList"][i]["startTime"], chartdata.bpm[i].startTime);
	}
	//META
	std::cout << "正在读取META\n";
	chartdata.meta.RPEVersion = data["META"]["RPEVersion"];
	chartdata.meta.background = data["META"]["background"];
	chartdata.meta.charter = data["META"]["charter"];
	chartdata.meta.composer = data["META"]["composer"];
	chartdata.meta.id = data["META"]["id"];
	chartdata.meta.illustration = data["META"]["illustration"];
	chartdata.meta.level = data["META"]["level"];
	chartdata.meta.name = data["META"]["name"];
	chartdata.meta.offset = data["META"]["offset"];
	chartdata.meta.song = data["META"]["song"];
	//Line
	std::cout << "正在读取Line\n";
	chartdata.lines.resize(data["judgeLineList"].size());
	judgeLine* temp_line;
	for (int i = 0; i < data["judgeLineList"].size(); i++)
	{
		std::cout << i << "\n";
		temp_line = &chartdata.lines[i];
		json jl = data["judgeLineList"][i];
		temp_line->Name = jl["Name"];
		temp_line->Texture = jl["Texture"];
		temp_line->bpmfactor = jl["bpmfactor"];
		temp_line->isCover = jl["isCover"];
		//eventLayers
		std::cout << "\teventLayers\n";
		temp_line->eventLayers.resize(jl["eventLayers"].size());
		for (int j = 0; j < jl["eventLayers"].size(); j++)
		{
			std::cout << "\t" << j << "\n";
			json el = jl["eventLayers"][j];
			//alphaEvents
			std::cout << "\t\talphaEvents " << el["alphaEvents"].size() << "\n";
			event_sum += el["alphaEvents"].size();
			temp_line->eventLayers[j].alphaEvents.resize(el["alphaEvents"].size());
			Readevent(el["alphaEvents"], temp_line->eventLayers[j].alphaEvents);
			//moveXEvents
			std::cout << "\t\tmoveXEvents " << el["moveXEvents"].size() << "\n";
			event_sum += el["moveXEvents"].size();
			temp_line->eventLayers[j].moveXEvents.resize(el["moveXEvents"].size());
			Readevent(el["moveXEvents"], temp_line->eventLayers[j].moveXEvents);
			//moveYEvents
			std::cout << "\t\tmoveYEvents " << el["moveYEvents"].size() << "\n";
			event_sum += el["moveYEvents"].size();
			temp_line->eventLayers[j].moveYEvents.resize(el["moveYEvents"].size());
			Readevent(el["moveYEvents"], temp_line->eventLayers[j].moveYEvents);
			//rotateEvents
			std::cout << "\t\trotateEvents " << el["rotateEvents"].size() << "\n";
			event_sum += el["rotateEvents"].size();
			temp_line->eventLayers[j].rotateEvents.resize(el["rotateEvents"].size());
			Readevent(el["rotateEvents"], temp_line->eventLayers[j].rotateEvents);
			//speedEvents
			std::cout << "\t\tspeedEvents " << el["speedEvents"].size() << "\n";
			event_sum += el["speedEvents"].size();
			temp_line->eventLayers[j].speedEvents.resize(el["speedEvents"].size());
			Readevent(el["speedEvents"], temp_line->eventLayers[j].speedEvents);
		}
		//notes
		std::cout << "\tnotes " << jl["notes"].size() << "\n";
		note_sum += jl["notes"].size();
		temp_line->notes.resize(jl["notes"].size());
		Readnote(jl["notes"], temp_line->notes);
	}
	std::cout << "event " << event_sum << "note " << note_sum << "\n";
	return chartdata;
}
