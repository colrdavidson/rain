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

typedef struct Queue {
	QueueNode *head;
	QueueNode *tail;
} Queue;

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

void free_sightlist(SightNode **head) {
	SightNode *tmp;
	while (*head != NULL) {
		tmp = *head;
		*head = (*head)->next;
		free(tmp);
	}
}

void print_sightlist(SightNode *head) {
	SightNode *tmp = head;
	while (tmp != NULL) {
		printf("(%d, %d, %d)\n", tmp->p.x, tmp->p.y, tmp->p.z);
		tmp = tmp->next;
	}
}

void push_node(SightNode **head, Point p) {
	SightNode *tmp = malloc(sizeof(SightNode));
	tmp->p = p;
	tmp->next = *head;

	*head = tmp;
}

void update_visible(Map *m, Entity *e, u8 visible) {
	if (e == NULL) {
		return;
	}

	Point p = e->pos;
	free_sightlist(&e->vis_head);

	i32 radius = e->sight_radius;
	for (u32 z = 0; z < m->depth; z++) {
		for (i32 y = -radius; y <= radius; y++) {
			for (i32 x = -radius; x <= radius; x++) {
				if ((x * x) + (y * y) <= (radius * radius)) {
					WorldSpace *s = get_map_space(m, p.x + x, p.y + y, z);
					if (s != NULL) {
						push_node(&e->vis_head, new_point(p.x + x, p.y + y, z));
						if (visible) {
							get_map_space(m, p.x + x, p.y + y, z)->visible = true;
						}
					}
				}
			}
		}
	}
}

u8 is_visible(Entity *e, Point p) {
	SightNode *tmp = e->vis_head;
	while (tmp != NULL) {
		if (point_eq(tmp->p, p)) {
			return true;
		}
	}

	return false;
}

void qpush(Queue *q, GridNode *tile) {
	QueueNode *tmp = malloc(sizeof(QueueNode));
	tmp->tile = tile;
	tmp->next = NULL;

	if (q->tail == NULL && q->head == NULL) {
		q->tail = tmp;
		q->head = q->tail;
	} else {
		if (q->head == q->tail) {
			q->tail->next = tmp;
			q->tail = tmp;
			q->head->next = q->tail;
		} else {
			q->tail->next = tmp;
			q->tail = tmp;
		}
	}
}

GridNode *qpop(Queue *q) {
	if (q->head == NULL && q->tail == NULL) {
		printf("empty queue!\n");
		return NULL;
	}

	QueueNode *tmp = q->head->next;
	GridNode *tile = q->head->tile;


	if (q->head == q->tail) {
		q->tail = NULL;
	}

	free(q->head);
	q->head = tmp;

	return tile;
}

void qprint(Queue *q) {
	QueueNode *tmp = q->head;
	while (tmp != NULL) {
		printf("%d\n", tmp->tile->tile_pos);
		tmp = tmp->next;
	}
}

Queue *new_queue() {
	Queue *q = (Queue *)malloc(sizeof(Queue));
	q->head = NULL;
	q->tail = NULL;
	return q;
}

void q_free(Queue *q) {
	while (q->head != NULL) {
		qpop(q);
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
	Queue *q = new_queue();
	GridNode **from = calloc(map_width * map_height * map_depth, sizeof(GridNode));

	qpush(q, &node_map[threed_to_oned(start.x, start.y, start.z, map_width, map_height)]);
	from[threed_to_oned(start.x, start.y, start.z, map_width, map_height)] = NULL;

	while (q->head != NULL && q->tail != NULL) {
		GridNode *cur_tile = qpop(q);

		if (cur_tile->tile_pos == threed_to_oned(goal.x, goal.y, goal.z, map_width, map_height)) {
			break;
		}

		for (u32 i = 0; i < max_neighbors; i++) {
			if (cur_tile->neighbors[i] != NULL && !from[cur_tile->neighbors[i]->tile_pos] && !cur_tile->neighbors[i]->w->solid) {
				qpush(q, cur_tile->neighbors[i]);

				from[cur_tile->neighbors[i]->tile_pos] = cur_tile;
			}
		}
	}

	PathNode *path_head = malloc(sizeof(PathNode));
	path_head->p = goal;
	path_head->next = NULL;

    while (!point_eq(path_head->p, start)) {
		GridNode *tmp = from[threed_to_oned(path_head->p.x, path_head->p.y, path_head->p.z, map_height, map_width)];
        if (tmp == NULL) {
			while (q->head != NULL) {
				qpop(q);
			}
			free(from);
			return NULL;
		}

		lappend(&path_head, oned_to_threed(tmp->tile_pos, map_width, map_height));
	}

	q_free(q);
	free(from);

	return path_head;
}

#endif
