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

void lprint(PathNode *head) {
	PathNode *tmp = head;
	while (tmp != NULL) {
		printf("(%u, %u, %u)\n", tmp->p.x, tmp->p.y, tmp->p.z);
		tmp = tmp->next;
	}
}

#endif
