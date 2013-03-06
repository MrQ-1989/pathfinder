#include "pathfinder.h"
#include <string.h>
void pathfinder_alloc(struct PathFinder *&finder, int node_num)
{
	finder = (PathFinder*) malloc(sizeof(struct PathFinder));
	finder->work_data_ = (Node*) malloc(sizeof(struct Node) * node_num);
	finder->work_heap_ = (Heap*) malloc(sizeof(struct Heap));
}

void pathfinder_free(struct PathFinder *finder)
{
	free(finder->work_data_);
	free(finder->work_heap_);
	free(finder);
}

int node_index(struct PathFinder *finder, Point point)
{
	int node_width = finder->node_width_;
	int node_height = finder->node_height_;

	int node_x_num = finder->node_max_column_;
	int node_y_num = finder->node_max_row_;

	int index = point.x_ / node_width + point.y_ / node_height * node_x_num;
	return index;
}

int node_distance(PathFinder *finder, int start_index, int end_index)
{
	int rx_start = start_index / finder->node_max_column_;
	int ry_start = start_index % finder->node_max_column_;

	int rx_end = end_index / finder->node_max_column_;
	int ry_end = end_index % finder->node_max_column_;

	return abs(rx_start - rx_end) + abs(ry_start - ry_end);
}

void pathfinder_init(struct PathFinder *finder, Json::Value &cfg)
{
	finder->node_width_ = cfg["node"]["nodeWidth"].asInt();
	finder->node_height_ = cfg["node"]["nodeHeight"].asInt();

	finder->node_max_column_ = cfg["node"]["nodeColumn"].asInt();
	finder->node_max_row_ = cfg["node"]["nodeRow"].asInt();

	finder->start_index_ = node_index(finder, finder->start_point_);
	finder->end_index_ = node_index(finder, finder->end_point_);

	for (int i = 0; i < finder->node_max_row_; i++)
	{
		for (int j = 0; j < finder->node_max_column_; j++)
		{
			int index = i * finder->node_max_column_ + j;
			finder->work_data_[index].index_ = index;
			finder->work_data_[index].data_ = *cfg["node"]["nodeAry"][index].asCString();
			finder->work_data_[index].f_ = 0;
			finder->work_data_[index].g_ = 0;
			finder->work_data_[index].h_ = 0;
			finder->work_data_[index].parent_index_ = 0;
			finder->work_data_[index].state_ = NS_UNKNOWN;
		}
	}

	memset(finder->work_heap_, 0, sizeof(struct Heap));

	finder->direction[0].xy_ = -finder->node_max_column_;
	finder->direction[0].column_index_ = -1;

	finder->direction[1].xy_ = 1;
	finder->direction[1].column_index_ = 0;

	finder->direction[2].xy_ = finder->node_max_column_;
	finder->direction[2].column_index_ = 1;

	finder->direction[3].xy_ = -1;
	finder->direction[3].column_index_ = 0;
}

void pathfinder_reset(struct PathFinder *finder)
{
	for (int i = 0; i < finder->node_max_row_; i++)
	{
		for (int j = 0; j < finder->node_max_column_; j++)
		{
			int index = i * finder->node_max_column_ + j;
			finder->work_data_[index].f_ = 0;
			finder->work_data_[index].g_ = 0;
			finder->work_data_[index].h_ = 0;
			finder->work_data_[index].parent_index_ = 0;
			finder->work_data_[index].state_ = NS_UNKNOWN;
		}
	}
	finder->work_heap_->last_leaf_ = 0;
}

void remove_from_heap(struct PathFinder *finder)
{
	finder->work_heap_->heap_[HEAP_ROOT] = finder->work_heap_->heap_[finder->work_heap_->last_leaf_--];
	int index = HEAP_ROOT;

	while (index <= finder->work_heap_->last_leaf_)
	{
		int left_index = LEFT(index);
		int right_index = RIGHT(index);
		int min_index = 0;

		if (left_index <= finder->work_heap_->last_leaf_ && right_index <= finder->work_heap_->last_leaf_)
		{
			if (finder->work_data_[left_index].f_ < finder->work_data_[right_index].f_)
				min_index = left_index;
			else
				min_index = right_index;
		}
		else if (left_index <= finder->work_heap_->last_leaf_)
			min_index = left_index;
		else
			break;

		if (finder->work_data_[finder->work_heap_->heap_[min_index]].f_ < finder->work_data_[finder->work_heap_->heap_[index]].f_)
		{
			int node_tmp = finder->work_heap_->heap_[min_index];
			finder->work_heap_->heap_[min_index] = finder->work_heap_->heap_[index];
			finder->work_heap_->heap_[index] = node_tmp;
			index = min_index;
		}
		else
			break;
	}
}

void insert_to_heap(struct PathFinder *finder, int node_index)
{
	if (finder->work_heap_->last_leaf_ < HEAP_SIZE)
	{
		finder->work_heap_->last_leaf_++;
	}
	finder->work_heap_->heap_[finder->work_heap_->last_leaf_] = node_index;

	int index = finder->work_heap_->last_leaf_;
	int parent = 0;
	while (index != 0)
	{
		parent = PARENT(index);
		if (parent != 0)
		{
			if (finder->work_data_[finder->work_heap_->heap_[index]].f_ < finder->work_data_[finder->work_heap_->heap_[parent]].f_)
			{
				int node_tmp = finder->work_heap_->heap_[parent];
				finder->work_heap_->heap_[parent] = finder->work_heap_->heap_[index];
				finder->work_heap_->heap_[index] = node_tmp;
				index = parent;
			}
			else
				break;
		}
		else
			break;
	}
}

int pathfinder_findparent(struct PathFinder *finder, int child_index)
{
	for (int i = 0; i < DIRECTION_SIZE; i++)
	{
		int parent_index = child_index + finder->direction[i].xy_;
		if (parent_index >= 0 && parent_index <= finder->node_max_column_ * finder->node_max_row_)
		{
			int start_index = (child_index / finder->node_max_column_ + finder->direction[i].column_index_) * finder->node_max_column_;
			int end_index = start_index + finder->node_max_column_ - 1;
			if (parent_index >= start_index && parent_index <= end_index)
			{
				if (finder->work_data_[parent_index].data_ == '0' && finder->work_data_[parent_index].state_ == NS_UNKNOWN)
				{
					finder->work_data_[parent_index].state_ = NS_OPEN;
					finder->work_data_[parent_index].g_ = node_distance(finder, parent_index, finder->end_index_);
					;
					finder->work_data_[parent_index].h_ = node_distance(finder, parent_index, finder->end_index_);
					finder->work_data_[parent_index].f_ = finder->work_data_[parent_index].g_ + finder->work_data_[parent_index].h_;
					finder->work_data_[parent_index].parent_index_ = child_index;
					insert_to_heap(finder, parent_index);
				}
			}

		}
	}
	return 0;
}

int pathfinder_packroute(struct PathFinder *finder, std::vector<int> &route_data)
{
	int index = finder->end_index_;
	for (; index != finder->start_index_;)
	{
		route_data.push_back(index);
		index = finder->work_data_[index].parent_index_;
	}
	route_data.push_back(index);
	return 0;
}

void pathfinder_findpath(struct PathFinder *finder, std::vector<int> &route_data)
{
	int start_index = finder->start_index_;
	finder->work_data_[start_index].state_ = NS_OPEN;
	finder->work_data_[start_index].index_ = finder->start_index_;
	finder->work_data_[start_index].g_ = 0;
	finder->work_data_[start_index].h_ = node_distance(finder, finder->start_index_, finder->end_index_);
	finder->work_data_[start_index].f_ = finder->work_data_[start_index].g_ + finder->work_data_[start_index].h_;

	insert_to_heap(finder, start_index);

	do
	{
		int node_index = finder->work_heap_->heap_[HEAP_ROOT];
		remove_from_heap(finder);

		finder->work_data_[node_index].state_ = NS_CLOSED;

		if (node_index == finder->end_index_)
		{
			pathfinder_packroute(finder, route_data);
			return;
		}

		pathfinder_findparent(finder, node_index);

	} while (true);

	return;
}
