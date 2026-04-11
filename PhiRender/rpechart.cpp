#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "cJSON.h"
#include "rpechart.h"

using namespace RPE;

void Readtime(cJSON* arr, int* time)
{
	for (int i = 0; i < 3; i++)
	{
		cJSON* temp = cJSON_GetArrayItem(arr, i);
		time[i] = cJSON_GetNumberValue(temp);
	}
}

void Readevent(cJSON* arr, std::vector<Event>& event)
{
	int size = cJSON_GetArraySize(arr);
	cJSON* arr_item, * item;
	for (int i = 0; i < size; i++)
	{
		arr_item = cJSON_GetArrayItem(arr, i);
		item = cJSON_GetObjectItem(arr_item, "easingType");
		event[i].easingType = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "end");
		event[i].end = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "endTime");
		Readtime(item, event[i].endTime);
		item = cJSON_GetObjectItem(arr_item, "linkgroup");
		event[i].linkgroup = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "start");
		event[i].start = cJSON_GetNumberValue(item);
		item = cJSON_GetObjectItem(arr_item, "startTime");
		Readtime(item, event[i].startTime);
	}
}

Chartdata RPE::Readdata(const char* filename)
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
	cJSON* arr1, * arr2, * arr3;
	int arr_size1, arr_size2, arr_size3;
	cJSON* arr_item1, * arr_item2;
	cJSON* item1, * item2;
	int event_sum = 0, note_sum = 0;
	//BPM
	printf("正在读取BPM\n");
	arr1 = cJSON_GetObjectItem(root, "BPMList");
	arr_size1 = cJSON_GetArraySize(arr1);
	chartdata.bpm.resize(arr_size1);
	for (int i = 0; i < arr_size1; i++)
	{
		arr_item1 = cJSON_GetArrayItem(arr1, i);
		item1 = cJSON_GetObjectItem(arr_item1, "bpm");
		chartdata.bpm[i].bpm = cJSON_GetNumberValue(item1);
		arr2 = cJSON_GetObjectItem(arr_item1, "startTime");
		Readtime(arr2, chartdata.bpm[i].startTime);
	}
	//META
	printf("正在读取META\n");
	item1 = cJSON_GetObjectItem(root, "META");
	item2 = cJSON_GetObjectItem(item1, "RPEVersion");
	chartdata.meta.RPEVersion = cJSON_GetNumberValue(item2);
	item2 = cJSON_GetObjectItem(item1, "background");
	chartdata.meta.background = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "charter");
	chartdata.meta.charter = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "composer");
	chartdata.meta.composer = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "id");
	chartdata.meta.id = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "illustration");
	chartdata.meta.illustration = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "level");
	chartdata.meta.level = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "name");
	chartdata.meta.name = _strdup(cJSON_GetStringValue(item2));
	item2 = cJSON_GetObjectItem(item1, "offset");
	chartdata.meta.offset = cJSON_GetNumberValue(item2);
	item2 = cJSON_GetObjectItem(item1, "song");
	chartdata.meta.song = _strdup(cJSON_GetStringValue(item2));
	//Line
	printf("正在读取Line\n");
	arr1 = cJSON_GetObjectItem(root, "judgeLineList");
	arr_size1 = cJSON_GetArraySize(arr1);
	chartdata.lines.resize(arr_size1);
	judgeLine* temp_line;
	for (int i = 0; i < arr_size1; i++)
	{
		printf("%d\n", i);
		temp_line = &chartdata.lines[i];
		arr_item1 = cJSON_GetArrayItem(arr1, i);
		item1 = cJSON_GetObjectItem(arr_item1, "Name");
		temp_line->Name = _strdup(cJSON_GetStringValue(item1));
		item1 = cJSON_GetObjectItem(arr_item1, "Texture");
		temp_line->Texture = _strdup(cJSON_GetStringValue(item1));
		item1 = cJSON_GetObjectItem(arr_item1, "bpmfactor");
		temp_line->bpmfactor = cJSON_GetNumberValue(item1);
		item1 = cJSON_GetObjectItem(arr_item1, "isCover");
		temp_line->isCover = cJSON_GetNumberValue(item1);
		//eventLayers
		printf("\teventLayers\n");
		arr2 = cJSON_GetObjectItem(arr_item1, "eventLayers");
		arr_size2 = cJSON_GetArraySize(arr2);
		temp_line->eventLayers.resize(arr_size2);
		for (int j = 0; j < arr_size2; j++)
		{
			arr_item2 = cJSON_GetArrayItem(arr2, j);
			arr3 = cJSON_GetObjectItem(arr_item2, "alphaEvents");
			arr_size3 = cJSON_GetArraySize(arr3);
			printf("\t\talphaEvents %d\n", arr_size3);
			event_sum += arr_size3;
			temp_line->eventLayers[j].alphaEvents.resize(arr_size3);
			Readevent(arr3, temp_line->eventLayers[j].alphaEvents);
			arr3 = cJSON_GetObjectItem(arr_item2, "moveXEvents");
			arr_size3 = cJSON_GetArraySize(arr3);
			printf("\t\tmoveXEvents %d\n", arr_size3);
			event_sum += arr_size3;
			temp_line->eventLayers[j].moveXEvents.resize(arr_size3);
			Readevent(arr3, temp_line->eventLayers[j].moveXEvents);
			arr3 = cJSON_GetObjectItem(arr_item2, "moveYEvents");
			arr_size3 = cJSON_GetArraySize(arr3);
			printf("\t\tmoveYEvents %d\n", arr_size3);
			event_sum += arr_size3;
			temp_line->eventLayers[j].moveYEvents.resize(arr_size3);
			Readevent(arr3, temp_line->eventLayers[j].moveYEvents);
			arr3 = cJSON_GetObjectItem(arr_item2, "rotateEvents");
			arr_size3 = cJSON_GetArraySize(arr3);
			printf("\t\trotateEvents %d\n", arr_size3);
			event_sum += arr_size3;
			temp_line->eventLayers[j].rotateEvents.resize(arr_size3);
			Readevent(arr3, temp_line->eventLayers[j].rotateEvents);
			arr3 = cJSON_GetObjectItem(arr_item2, "speedEvents");
			arr_size3 = cJSON_GetArraySize(arr3);
			printf("\t\tspeedEvents %d\n", arr_size3);
			event_sum += arr_size3;
			temp_line->eventLayers[j].speedEvents.resize(arr_size3);
			Readevent(arr3, temp_line->eventLayers[j].speedEvents);
		}
		//notes
		arr2 = cJSON_GetObjectItem(arr_item1, "notes");
		arr_size2 = cJSON_GetArraySize(arr2);
		printf("\tnotes %d\n", arr_size2);
		note_sum += arr_size2;
		temp_line->notes.resize(arr_size2);
		Note* temp_note;
		for (int j = 0; j < arr_size2; j++)
		{
			temp_note = &temp_line->notes[j];
			arr_item2 = cJSON_GetArrayItem(arr2, j);
			item1 = cJSON_GetObjectItem(arr_item2, "above");
			temp_note->above = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "alpha");
			temp_note->alpha = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "endTime");
			Readtime(item1, temp_note->endTime);
			item1 = cJSON_GetObjectItem(arr_item2, "isFake");
			temp_note->isFake = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "positionX");
			temp_note->positionX = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "size");
			temp_note->size = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "speed");
			temp_note->speed = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "startTime");
			Readtime(item1, temp_note->startTime);
			item1 = cJSON_GetObjectItem(arr_item2, "type");
			temp_note->type = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "visibleTime");
			temp_note->visibleTime = cJSON_GetNumberValue(item1);
			item1 = cJSON_GetObjectItem(arr_item2, "yOffset");
			temp_note->yOffset = cJSON_GetNumberValue(item1);
		}
	}

	printf("event %d note %d\n", event_sum, note_sum);

	cJSON_Delete(root);
	return chartdata;
}

void RPE::Printdata(Chartdata data)
{
	printf("data:\n");
	for (int i = 0; i < data.lines.size(); i++)
	{
		printf("line %d\n", i);
		printf("note %llu\n", data.lines[i].notes.size());
		printf("eventLayer %llu\n", data.lines[i].eventLayers.size());
		for (int j = 0; j < data.lines[i].eventLayers.size(); j++)
		{
			printf("layer %d\n", j);
			printf("event %llu\n", data.lines[i].eventLayers[j].alphaEvents.size() + \
				data.lines[i].eventLayers[j].moveXEvents.size() + \
				data.lines[i].eventLayers[j].moveYEvents.size() + \
				data.lines[i].eventLayers[j].rotateEvents.size() + \
				data.lines[i].eventLayers[j].speedEvents.size());
		}
	}
}

void RPE::AddLine(Chartdata* data)
{
	judgeLine newline;
	newline.Name = _strdup("Default");
	newline.Texture = _strdup("line.png");
	newline.bpmfactor = 1.0;
	newline.isCover = 1;
	data->lines.push_back(newline);
}

void RPE::AddNote(Chartdata* data, int type, int line, int starttime[], int endtime[], int above, float speed, float positionX)
{
	Note newnote;
	newnote.above = above;
	newnote.alpha = 255;
	newnote.isFake = 0;
	newnote.size = 1.0;
	newnote.speed = speed;
	newnote.visibleTime = 999999.0;
	newnote.yOffset = 0.0;
	newnote.type = type;
	newnote.positionX = positionX;
	newnote.startTime[0] = starttime[0];
	newnote.startTime[1] = starttime[1];
	newnote.startTime[2] = starttime[2];
	newnote.endTime[0] = endtime[0];
	newnote.endTime[1] = endtime[1];
	newnote.endTime[2] = endtime[2];
	data->lines[line].notes.push_back(newnote);
}

void RPE::AddEvent(Chartdata* data, char* eventtype, int line, int starttime[], int endtime[], int start[], int end[], int easingtype)
{

}