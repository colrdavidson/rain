#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include "common.h"
#include "point.h"

typedef struct Entity {
	Direction dir;
    u8 cur_health;
    u8 max_health;
	u8 attack_damage;
	u32 sprite_id;
} Entity;

typedef struct Tile {
	u32 sprite_id;
} Tile;

typedef struct WorldSpace {
	Entity *e;
	Tile *t;
	u8 solid;
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
		s->solid = 0;

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
	tmp->dir = dir;

	return tmp;
}

WorldSpace *get_map_space(Map *m, u32 x, u32 y, u32 z) {
 	WorldSpace *ret = m->spaces[threed_to_oned(x, y, z, m->width, m->height)];
	return ret;
}

Tile *get_map_tile(Map *m, u32 x, u32 y, u32 z) {
	Tile *ret = get_map_space(m, x, y, z)->t;
	return ret;
}

Entity *get_map_entity(Map *m, u32 x, u32 y, u32 z) {
 	Entity *ret = get_map_space(m, x, y, z)->e;
	return ret;
}

void set_map_space(Map *m, u32 x, u32 y, u32 z, WorldSpace *w) {
 	WorldSpace *tmp = get_map_space(m, x, y, z);
	m->spaces[threed_to_oned(x, y, z, m->width, m->height)] = w;

	if (tmp != NULL) {
		free(tmp);
	}
}

void set_map_tile(Map *m, u32 x, u32 y, u32 z, Tile *t, u8 solid) {
 	Tile *tmp = get_map_tile(m, x, y, z);
	m->spaces[threed_to_oned(x, y, z, m->width, m->height)]->t = t;
	m->spaces[threed_to_oned(x, y, z, m->width, m->height)]->solid = solid;

	if (tmp != NULL) {
		free(tmp);
	}
}

void set_map_entity(Map *m, u32 x, u32 y, u32 z, Entity *e) {
 	Entity *tmp = get_map_entity(m, x, y, z);
	m->spaces[threed_to_oned(x, y, z, m->width, m->height)]->e = e;

	if (e == NULL) {
		m->spaces[threed_to_oned(x, y, z, m->width, m->height)]->solid = 0;
	} else {
		m->spaces[threed_to_oned(x, y, z, m->width, m->height)]->solid = 1;
	}

	if (tmp != NULL) {
		free(tmp);
	}
}


u8 is_space_empty(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_tile(m, x, y, z) == NULL && get_map_entity(m, x, y, z) == NULL) {
		return 1;
	}
	return 0;
}

u8 has_tile(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_tile(m, x, y, z) != NULL) {
		return 1;
	}

	return 0;
}

u8 has_entity(Map *m, u32 x, u32 y, u32 z) {
	if (get_map_entity(m, x, y, z) != NULL) {
		return 1;
	}

	return 0;
}

u8 can_move(Map *m, Point self, Point goal) {
	if (has_entity(m, self.x, self.y, self.z) && !get_map_space(m, goal.x, goal.y, goal.z)->solid) {
		return 1;
	}

	return 0;
}

void move_entity(Map *m, Point a, Point b) {
	if (has_entity(m, b.x, b.y, b.z)) {
		printf("(%u, %u, %u) FULL!\n", b.x, b.y, b.z);
		return;
	}

	Entity *tmp = get_map_entity(m, a.x, a.y, a.z);
	set_map_entity(m, b.x, b.y, b.z, tmp);

	m->spaces[threed_to_oned(a.x, a.y, a.z, m->width, m->height)]->e = NULL;
	m->spaces[threed_to_oned(a.x, a.y, a.z, m->width, m->height)]->solid = 0;

}


#endif
