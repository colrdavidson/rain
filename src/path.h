#ifndef PATH_H
#define PATH_H

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "point.h"

typedef struct GridNode {
	u32 tile_id;
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
	GridNode *tile = (*head)->tile;
	QueueNode *tmp = (*head)->next;

	free(*head);
	*head = tmp;

	return tile;
}

void qprint(QueueNode *head) {
	QueueNode *tmp = head;
	while (tmp != NULL) {
		printf("%d\n", tmp->tile->tile_id);
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

void lprint(PathNode *head) {
	PathNode *tmp = head;
	while (tmp != NULL) {
		printf("(%u, %u, %u)\n", tmp->p.x, tmp->p.y, tmp->p.z);
		tmp = tmp->next;
	}
}

PathNode *find_path(Point start, Point goal, GridNode *node_map, u32 map_width, u32 map_height, u32 max_neighbors) {
	QueueNode *head = NULL;
	QueueNode *tail = NULL;
	GridNode **from = malloc(map_width * map_height * sizeof(GridNode));

	qpush(&head, &tail, &node_map[twod_to_oned(start.x, start.y, map_width)]);
	from[twod_to_oned(start.x, start.y, map_width)] = NULL;

	while (head != NULL && tail != NULL) {
		GridNode *cur_tile = qpop(&head);

		if (cur_tile->tile_id == twod_to_oned(goal.x, goal.y, map_width)) {
			break;
		}

		for (u32 i = 0; i < max_neighbors; i++) {
			if (cur_tile->neighbors[i] != NULL && !from[cur_tile->neighbors[i]->tile_id]) {
				qpush(&head, &tail, cur_tile->neighbors[i]);
				from[cur_tile->neighbors[i]->tile_id] = cur_tile;
			}
		}
	}

	PathNode *path_head = malloc(sizeof(PathNode));
	path_head->p = goal;
	path_head->next = NULL;

    while (point_eq(path_head->p, start)) {
		lappend(&path_head, oned_to_twod(from[twod_to_oned(path_head->p.x, path_head->p.y, map_height)]->tile_id, map_width));
	}

	return path_head;
}

#endif
