#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "point.h"
#include "map.h"

typedef struct SightNode {
    Point p;
	struct SightNode *next;
} SightNode;

typedef struct Entity {
	Direction dir;
    i8 cur_health;
    u8 max_health;
	u8 attack_damage;
	u8 turn_points;
	u32 sprite_id;
	u8 sight_radius;
	u8 moved;
	SightNode *vis_head;
	Point pos;
} Entity;

typedef struct Tile {
	u32 sprite_id;
} Tile;

typedef struct WorldSpace {
	Entity *e;
	Tile *t;
	u8 solid;
	u8 visible;
} WorldSpace;

typedef struct Map {
	WorldSpace **spaces;
	u32 width;
	u32 height;
	u32 depth;
	u32 size;
} Map;

Map *new_map(u32 width, u32 height, u32 depth) {
	Map *m = malloc(sizeof(Map));
	m->width = width;
	m->height = height;
	m->depth = depth;
	m->size = width * height * depth;
	m->spaces = malloc(m->size * sizeof(WorldSpace *));

	for (u32 i = 0; i < m->size; i++) {
		WorldSpace *s = malloc(sizeof(WorldSpace));
		s->t = NULL;
		s->e = NULL;
		s->solid = false;
		s->visible = false;

		m->spaces[i] = s;
	}

	return m;
}

Tile *new_tile(u32 sprite_id) {
	Tile *tmp = malloc(sizeof(Tile));
	tmp->sprite_id = sprite_id;

	return tmp;
}

Entity *new_entity(u32 sprite_id, u32 attack_damage, u32 health, Direction dir) {
	Entity *tmp = malloc(sizeof(Entity));
	tmp->sprite_id = sprite_id;
	tmp->cur_health = health;
	tmp->max_health = health;
	tmp->attack_damage = attack_damage;
	tmp->turn_points = 2;
	tmp->dir = dir;
	tmp->sight_radius = 6;
	tmp->moved = true;
	tmp->vis_head = NULL;
	tmp->pos = new_point(0, 0, 0);

	return tmp;
}

WorldSpace *get_map_space(Map *m, u32 x, u32 y, u32 z) {
	if (x < m->width && y < m->height && z < m->depth) {
		WorldSpace *ret = m->spaces[threed_to_oned(x, y, z, m->width, m->height)];
		return ret;
	} else {
		return NULL;
	}
}

void set_map_space(Map *m, u32 x, u32 y, u32 z, WorldSpace *w) {
 	WorldSpace *tmp = get_map_space(m, x, y, z);
	m->spaces[threed_to_oned(x, y, z, m->width, m->height)] = w;

	if (tmp != NULL) {
		free(tmp);
	}
}

Tile *get_map_tile(Map *m, u32 x, u32 y, u32 z) {
	Tile *ret = get_map_space(m, x, y, z)->t;
	return ret;
}

void set_map_tile(Map *m, u32 x, u32 y, u32 z, Tile *t, u8 solid) {
 	Tile *tmp = get_map_tile(m, x, y, z);
	get_map_space(m, x, y, z)->t = t;
	get_map_space(m, x, y, z)->solid = solid;

	if (tmp != NULL) {
		free(tmp);
	}
}

Entity *get_map_entity(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_space(m, x, y, z) == NULL) {
		return NULL;
	} else {
		Entity *ret = get_map_space(m, x, y, z)->e;
		return ret;
	}
}

void set_map_entity(Map *m, u32 x, u32 y, u32 z, Entity *e) {
 	Entity *tmp = get_map_entity(m, x, y, z);
	get_map_space(m, x, y, z)->e = e;

	if (e == NULL) {
		get_map_space(m, x, y, z)->solid = false;
	} else {
		get_map_space(m, x, y, z)->solid = true;
		e->pos = new_point(x, y, z);
	}

	if (tmp != NULL) {
		free(tmp);
	}
}

u8 is_valid_space(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_space(m, x, y, z) != NULL) {
		return true;
	}
	return false;
}

u8 is_space_empty(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_tile(m, x, y, z) == NULL && get_map_entity(m, x, y, z) == NULL) {
		return true;
	}
	return false;
}

u8 has_tile(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_tile(m, x, y, z) != NULL) {
		return true;
	}

	return false;
}

u8 has_entity(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_entity(m, x, y, z) != NULL) {
		return true;
	}

	return false;
}

u8 can_move(Map *m, Point self, Point goal) {
	if (has_entity(m, self.x, self.y, self.z) && is_valid_space(m, goal.x, goal.y, goal.z) && !get_map_space(m, goal.x, goal.y, goal.z)->solid && get_map_entity(m, self.x, self.y, self.z)->turn_points > 0) {
		return true;
	}

	return false;
}

void move_entity(Map *m, Point a, Point b) {
	if (has_entity(m, b.x, b.y, b.z)) {
		printf("(%u, %u, %u) FULL!\n", b.x, b.y, b.z);
		return;
	}

	Entity *tmp = get_map_entity(m, a.x, a.y, a.z);
	tmp->moved = true;

	set_map_entity(m, b.x, b.y, b.z, tmp);

	get_map_space(m, a.x, a.y, a.z)->e = NULL;
	get_map_space(m, a.x, a.y, a.z)->solid = false;
}

void print_entity_map(Entity **entity_map, u32 map_length) {
	for (u32 i = 0; i < map_length; i++) {
		printf("[PEM-%d] %p\n", i, entity_map[i]);
	}
}

u8 fire_at_entity(Map *m, Entity **entity_map, u32 entity_map_length, Entity *self, Entity *target, Mix_Chunk *shoot_wav) {
	u32 turns = self->turn_points;
	if (turns == 1) {
		self->turn_points = 0;
	} else if (turns > 1) {
		self->turn_points -= 2;
	} else {
		return false;
	}

	Mix_PlayChannel(-1, shoot_wav, 0);

	srand(time(NULL));
	u8 damage = (rand() % (self->attack_damage)) + 1;
	target->cur_health -= damage;
	if (target->cur_health <= 0) {
		for (u32 i = 0; i < entity_map_length; i++) {
			if (target == entity_map[i]) {
				entity_map[i] = NULL;
				break;
			}
		}

		set_map_entity(m, target->pos.x, target->pos.y, target->pos.z, NULL);
		return true;
	}

	return false;
}

#endif
