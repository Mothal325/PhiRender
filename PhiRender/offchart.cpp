#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "cJSON.h"
#include "offchart.h"

using namespace OFF;

void Readnote(cJSON* arr, std::vector<Note>& note)
{
	int size = cJSON_GetArraySize(arr);
	cJSON* arr_item, * item;
	for (int i = 0; i < size; i++)
	{
		arr_item = cJSON_GetArrayItem(arr, i);
		item = cJSON_GetObjectItem(arr_item, "type");
		note[i].type = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "time");
		note[i].time = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "positionX");
		note[i].positionX = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "holdTime");
		note[i].holdTime = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "speed");
		note[i].speed = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "floorPosition");
		note[i].floorPosition = cJSON_GetNumberValue(item);
	}
}

void Readevent(cJSON* arr, std::vector<Event>& event, int mode)	//mode: speed 0, move 1, another 2
{
	int size = cJSON_GetArraySize(arr);
	cJSON* arr_item, * item;
	for (int i = 0; i < size; i++)
	{
		arr_item = cJSON_GetArrayItem(arr, i);
		item = cJSON_GetObjectItem(arr_item, "startTime");
		event[i].startTime = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "endTime");
		event[i].endTime = cJSON_GetNumberValue(item);
		if (mode == 0)
		{
			item = cJSON_GetObjectItem(arr_item, "value");
			event[i].start = cJSON_GetNumberValue(item);
		}
		else
		{
			item = cJSON_GetObjectItem(arr_item, "start");
			event[i].start = cJSON_GetNumberValue(item);
			item = cJSON_GetObjectItem(arr_item, "end");
			event[i].end = cJSON_GetNumberValue(item);
		}
		if (mode == 1)
		{
			item = cJSON_GetObjectItem(arr_item, "start2");
			event[i].start2 = cJSON_GetNumberValue(item);
			item = cJSON_GetObjectItem(arr_item, "end2");
			event[i].end2 = cJSON_GetNumberValue(item);
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

Chartdata OFF::Readdata(const char* filename)
{
	FILE* fp;
	//打开文件
	fopen_s(&fp, filename, "r");
	if (fp == nullptr)
	{
		printf("文件打不开喵\n");
		exit(-1);
	}
	//确定文件大小
	_fseeki64(fp, 0, SEEK_END);
	__int64 filesize = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);
	if (filesize == 0)
	{
		printf("文件有问题喵\n");
		exit(-1);
	}
	//读取文件内容
	char* data = (char*)malloc(filesize * sizeof(char));
	if (data == nullptr)
	{
		printf("data内存申请失败喵\n");
		exit(-1);
	}
	fread_s(data, filesize, sizeof(char), filesize, fp);
	fclose(fp);
	cJSON* root = cJSON_Parse(data);
	free(data);
	if (root == nullptr)
	{
		printf("JSON解析错误喵\n");
		exit(-1);
	}
	//解析
	Chartdata chartdata;
	cJSON* arr1, * arr2;
	int arr_size1, arr_size2;
	cJSON* arr_item1, * arr_item2;
	cJSON* item1;
	int event_sum = 0, note_sum = 0;
	//基础信息
	item1 = cJSON_GetObjectItem(root, "formatVersion");
	chartdata.formatVersion = cJSON_GetNumberValue(item1);
	item1 = cJSON_GetObjectItem(root, "offset");
	chartdata.offset = cJSON_GetNumberValue(item1);
	//judgeLineList
	printf("正在读取judgeLineList\n");
	arr1 = cJSON_GetObjectItem(root, "judgeLineList");
	arr_size1 = cJSON_GetArraySize(arr1);
	chartdata.lines.resize(arr_size1);
	for (int i = 0; i < arr_size1; i++)
	{
		printf("%d\n", i);
		arr_item1 = cJSON_GetArrayItem(arr1, i);
		item1 = cJSON_GetObjectItem(arr_item1, "bpm");
		chartdata.lines[i].bpm = cJSON_GetNumberValue(item1);
		arr2 = cJSON_GetObjectItem(arr_item1, "notesAbove");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\tnotesAbove %d ", arr_size2);
		note_sum += arr_size2;
		chartdata.lines[i].notesAbove.resize(arr_size2);
		Readnote(arr2, chartdata.lines[i].notesAbove);
		arr2 = cJSON_GetObjectItem(arr_item1, "notesBelow");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("notesBelow %d\n", arr_size2);
		note_sum += arr_size2;
		chartdata.lines[i].notesBelow.resize(arr_size2);
		Readnote(arr2, chartdata.lines[i].notesBelow);
		arr2 = cJSON_GetObjectItem(arr_item1, "speedEvents");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\tspeedEvent %d\n", arr_size2);
		event_sum += arr_size2;
		chartdata.lines[i].speedEvents.resize(arr_size2);
		Readevent(arr2, chartdata.lines[i].speedEvents, 0);
		chartdata.lines[i].floorEvents.resize(arr_size2);
		CalculateFloor(chartdata.lines[i]);
		arr2 = cJSON_GetObjectItem(arr_item1, "judgeLineMoveEvents");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\tmoveEvents %d\n", arr_size2);
		event_sum += arr_size2;
		chartdata.lines[i].moveEvents.resize(arr_size2);
		Readevent(arr2, chartdata.lines[i].moveEvents, 1);
		arr2 = cJSON_GetObjectItem(arr_item1, "judgeLineRotateEvents");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\trotateEvents %d\n", arr_size2);
		event_sum += arr_size2;
		chartdata.lines[i].rotateEvents.resize(arr_size2);
		Readevent(arr2, chartdata.lines[i].rotateEvents, 1);
		arr2 = cJSON_GetObjectItem(arr_item1, "judgeLineDisappearEvents");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\tdisappearEvents %d\n", arr_size2);
		event_sum += arr_size2;
		chartdata.lines[i].disappearEvents.resize(arr_size2);
		Readevent(arr2, chartdata.lines[i].disappearEvents, 1);
	}

	printf("event %d note %d\n", event_sum, note_sum);
	cJSON_Delete(root);

	return chartdata;
}

void OFF::Printdata(Chartdata data)
{
	printf("data:\n");
	for (int i = 0; i < data.lines.size(); i++)
	{
		printf("line %d\n", i);
		printf("noteA %llu\n", data.lines[i].notesAbove.size());
		printf("noteB %llu\n", data.lines[i].notesBelow.size());
		printf("event %llu\n", data.lines[i].disappearEvents.size() + \
			data.lines[i].moveEvents.size() + \
			data.lines[i].rotateEvents.size() + \
			data.lines[i].speedEvents.size());
	}
}

void OFF::FindLine(judgeLine line, float time, Linedata& data)
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