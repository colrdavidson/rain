#ifndef PATH_H
#define PATH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "point.h"
#include "map.h"

typedef struct GridNode {
	u32 tile_pos;
	WorldSpace *w;
	struct GridNode **neighbors;
} GridNode;

typedef struct QueueNode {
	GridNode *tile;
	struct QueueNode *next;
} QueueNode;

typedef struct PathNode {
	Point p;
	struct PathNode *next;
} PathNode;

GridNode *new_nodemap(Map *map, u32 max_neighbors) {
	GridNode *node_map = malloc(sizeof(GridNode) * map->size);
	for (u32 x = 0; x < map->width; x++) {
		for (u32 y = 0; y < map->height; y++) {
			for (u32 z = 0; z < map->depth; z++) {
				GridNode tmp;
				tmp.tile_pos = threed_to_oned(x, y, z, map->width, map->height);
				tmp.w = get_map_space(map, x, y, z);

				tmp.neighbors = malloc(sizeof(GridNode) * max_neighbors);

				node_map[threed_to_oned(x, y, z, map->width, map->height)] = tmp;
			}
		}
	}

	for (u32 y = 0; y < map->height; y++) {
    	for (u32 x = 0; x < map->width; x++) {
    		for (u32 z = 0; z < map->depth; z++) {
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTH] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[EAST] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[WEST] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTH] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[UP] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[DOWN] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHEAST] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHWEST] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHEAST] = NULL;
				node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHWEST] = NULL;

				if (y < map->height - 1) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTH] = &node_map[threed_to_oned(x, y + 1, z, map->width, map->height)];
				}
				if (x < map->width - 1) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[EAST] = &node_map[threed_to_oned(x + 1, y, z, map->width, map->height)];
				}
				if (x > 0) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[WEST] = &node_map[threed_to_oned(x - 1, y, z, map->width, map->height)];
				}
				if (y > 0) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTH] = &node_map[threed_to_oned(x, y - 1, z, map->width, map->height)];
				}

				if (x > 0 && y > 0) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHWEST] = &node_map[threed_to_oned(x - 1, y - 1, z, map->width, map->height)];
				}

				if (x < map->width - 1 && y > 0) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHEAST] = &node_map[threed_to_oned(x + 1, y - 1, z, map->width, map->height)];
				}

				if (x < map->width - 1 && y < map->height - 1) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHWEST] = &node_map[threed_to_oned(x + 1, y + 1, z, map->width, map->height)];
				}

				if (x > 0 && y < map->height - 1) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHEAST] = &node_map[threed_to_oned(x - 1, y + 1, z, map->width, map->height)];
				}

				if (z > 0 && !has_tile(map, x, y, z - 1)) {
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTH] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[EAST] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[WEST] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTH] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[UP] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[DOWN] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHEAST] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[NORTHWEST] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHEAST] = NULL;
					node_map[threed_to_oned(x, y, z, map->width, map->height)].neighbors[SOUTHWEST] = NULL;
				}


				/*
				 * Fixed ID's are no longer useful in the new dynamic loader. To re-enable ladders,
				 * map files need to be able to denote block descriptors
				 *
				if (z < map_depth - 2 && map[threed_to_oned(x, y, z + 1, map_width, map_height)] == 9) {
					node_map[threed_to_oned(x, y - 1, z, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[UP] = &node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)];

					node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z, map_width, map_height)];
					node_map[threed_to_oned(x, y - 1, z + 2, map_width, map_height)].neighbors[DOWN] = &node_map[threed_to_oned(x, y - 1, z + 1, map_width, map_height)];
				}
				*/
			}
		}
	}

	return node_map;
}

void qpush(QueueNode **head, QueueNode **tail, GridNode *tile) {
	QueueNode *tmp = malloc(sizeof(QueueNode));
	tmp->tile = tile;
	tmp->next = NULL;

	if (*tail == NULL) {
		*tail = tmp;
	} else {
		(*tail)->next = tmp;
		*tail = tmp;
	}

	if (*head == NULL) {
		*head = *tail;
	}
}

GridNode *qpop(QueueNode **head) {
	if (*head == NULL) {
		printf("empty queue!\n");
		return NULL;
	}

	QueueNode *tmp = (*head)->next;
	GridNode *tile = (*head)->tile;

	free(*head);
	*head = tmp;

	return tile;
}

void qprint(QueueNode *head) {
	QueueNode *tmp = head;
	while (tmp != NULL) {
		printf("%d\n", tmp->tile->tile_pos);
		tmp = tmp->next;
	}
}

void lappend(PathNode **head, Point p) {
	PathNode *tmp = malloc(sizeof(PathNode));
	tmp->p = p;
	tmp->next = *head;

	*head = tmp;
}

void lreverse(PathNode **head) {
	PathNode *prev = NULL;
	PathNode *cur = *head;
	PathNode *next;

	while (cur != NULL) {
		next = cur->next;
		cur->next = prev;
		prev = cur;
		cur = next;
	}

	*head = prev;
}

void free_path(PathNode *head) {
	PathNode *tmp;
	while (head != NULL) {
		tmp = head;
		head = head->next;
		free(tmp);
	}
}

void lprint(PathNode *head) {
	PathNode *tmp = head;
	while (tmp != NULL) {
		printf("(%u, %u, %u)\n", tmp->p.x, tmp->p.y, tmp->p.z);
		tmp = tmp->next;
	}
}

PathNode *find_path(Point start, Point goal, GridNode *node_map, u32 map_width, u32 map_height, u32 map_depth, u32 max_neighbors) {
	QueueNode *head = NULL;
	QueueNode *tail = NULL;
	GridNode **from = malloc(map_width * map_height * map_depth * sizeof(GridNode));
	memset(from, 0, map_width * map_height * map_depth * sizeof(GridNode));

	qpush(&head, &tail, &node_map[threed_to_oned(start.x, start.y, start.z, map_width, map_height)]);
	from[threed_to_oned(start.x, start.y, start.z, map_width, map_height)] = NULL;

	while (head != NULL && tail != NULL) {
		GridNode *cur_tile = qpop(&head);

		if (cur_tile->tile_pos == threed_to_oned(goal.x, goal.y, goal.z, map_width, map_height)) {
			break;
		}

		for (u32 i = 0; i < max_neighbors; i++) {
			if (cur_tile->neighbors[i] != NULL && !from[cur_tile->neighbors[i]->tile_pos] && !cur_tile->neighbors[i]->w->solid) {
				qpush(&head, &tail, cur_tile->neighbors[i]);

				from[cur_tile->neighbors[i]->tile_pos] = cur_tile;
			}
		}
	}

	PathNode *path_head = malloc(sizeof(PathNode));
	path_head->p = goal;
	path_head->next = NULL;

    while (point_eq(path_head->p, start)) {
		GridNode *tmp = from[threed_to_oned(path_head->p.x, path_head->p.y, path_head->p.z, map_height, map_width)];
        if (tmp == NULL) {
			while (head != NULL) {
				qpop(&head);
			}
			free(from);
			return NULL;
		}

		lappend(&path_head, oned_to_threed(tmp->tile_pos, map_width, map_height));
	}

	while (head != NULL) {
		qpop(&head);
	}
	free(from);

	return path_head;
}

#endif
