#ifndef PATHFINDER_H_
#define PATHFINDER_H_

#include "./json/json.h"
#include <stdio.h>
#include <vector>
#include <algorithm>

#define LEFT(X) (X << 1)
#define RIGHT(X) ((X << 1) + 1)
#define PARENT(X) (X >> 1)

enum
{
	HEAP_ROOT = 1, HEAP_SIZE = 1024
};

enum NodeState
{
	NS_CLOSED, NS_OPEN, NS_UNKNOWN
};

enum
{
	DIRECTION_SIZE = 4
};

struct Node
{
	NodeState state_;
	int index_;
	int parent_index_;
	int f_;
	int g_;
	int h_;
	char data_;
};

struct Heap
{
	int heap_[HEAP_SIZE];
	int last_leaf_;
};

struct Point
{
	int x_;
	int y_;
};

struct Direction
{
	int xy_;
	int column_index_;
};

struct PathFinder
{
	Node *work_data_;
	Heap *work_heap_;

	Point start_point_;
	Point end_point_;

	int start_index_;
	int end_index_;

	int node_width_;
	int node_height_;

	int node_max_column_;
	int node_max_row_;

	Direction direction[DIRECTION_SIZE];
};

void pathfinder_alloc(struct PathFinder *&finder, int node_num);
void pathfinder_free(struct PathFinder *finder);
void pathfinder_init(struct PathFinder *finder, Json::Value &cfg);
void pathfinder_reset(struct PathFinder *finder);
void pathfinder_findpath(PathFinder *finder, std::vector<int> &route_data);
#endif
