//============================================================================
// Name        : main.cpp
// Author      : mrq
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <./json/json.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "pathfinder.h"

#define ERROR_RETURN(X,R) \
	do \
		{ \
			printf X;\
			return R;\
		} while (false);

int load_config(std::string file_name, Json::Value &cfg)
{
	FILE *file = fopen(file_name.c_str(), "rb");

	if (file == 0)
	{
		ERROR_RETURN(("failed to open file %s.\n",file_name.c_str()), -1);
	}

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	std::string config_doc = "";
	char *buffer = new char[size + 1];
	buffer[size] = 0;

	if (fread(buffer, 1, size, file) == (unsigned long) size)
	{
		config_doc = buffer;
	}

	fclose(file);
	delete[] buffer;

	Json::Reader reader;
	if (reader.parse(config_doc, cfg) == false)
	{
		ERROR_RETURN( ("failed to parse file %s\n error: %s\n context: %s.\n", file_name.c_str(), reader.getFormatedErrorMessages().c_str(), config_doc.c_str()), -1);
	}

	return 0;
}

int main()
{
	Json::Value map_cfg;
	
    if (load_config("./map.json", map_cfg) < 0)
        return -1;

	int node_num = map_cfg["node"]["nodeColumn"].asInt() * map_cfg["node"]["nodeRow"].asInt();
	PathFinder *finder;
	pathfinder_alloc(finder, node_num);

	finder->start_point_.x_ = 25;
	finder->start_point_.y_ = 25;

	finder->end_point_.x_ = 310;
	finder->end_point_.y_ = 340;

	pathfinder_init(finder, map_cfg);

	std::vector<int> route_data;
	pathfinder_findpath(finder, route_data);

	for (int i = 0; i < finder->node_max_row_; i++)
	{
		for (int j = 0; j < finder->node_max_column_; j++)
		{
			int index = j + i * finder->node_max_column_;
			if (finder->work_data_[index].data_ == '1')
			{
				printf("1 ");
				continue;
			}
			int flag = false;
			for (int k = 0; k < (int) route_data.size(); k++)
			{
				if ((j + i * finder->node_max_column_) == route_data[k])
				{
					printf("9 ");
					flag = true;
					break;
				}
			}
			if (flag == false)
			{
				printf("0 ");
			}
		}
		printf("\n");

	}

	return 0;
}
