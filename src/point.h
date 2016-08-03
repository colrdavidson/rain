#ifndef POINT_H
#define POINT_H

#include <stdio.h>
#include "common.h"

typedef struct Point {
	u32 x;
	u32 y;
	u32 z;
} Point;

typedef enum Direction {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NORTHEAST,
	NORTHWEST,
	SOUTHEAST,
	SOUTHWEST,
	UP,
	DOWN,
} Direction;

u32 threed_to_oned(u32 x, u32 y, u32 z, u32 x_max, u32 y_max) {
	return (z * x_max * y_max) + (y * x_max) + x;
}

u32 point_to_oned(Point p, u32 x_max, u32 y_max) {
	return (p.z * x_max * y_max) + (p.y * x_max) + p.x;
}

u32 twod_to_oned(u32 x, u32 y, u32 x_max) {
	return (y * x_max) + x;
}

Point oned_to_twod(u32 idx, u32 x_max) {
	Point p;

	p.x = idx % x_max;
	p.y = idx / x_max;
	p.z = 0;

	return p;
}

Point oned_to_threed(u32 idx, u32 x_max, u32 y_max) {
	u32 z = idx / (x_max * y_max);
	u32 tmp_idx = idx - (z * x_max * y_max);

	Point p = oned_to_twod(tmp_idx, x_max);
	p.z = z;

	return p;
}

Direction cycle_right(Direction dir) {
	switch (dir) {
		case NORTH: {
			return EAST;
		} break;
		case EAST: {
			return SOUTH;
		} break;
		case SOUTH: {
			return WEST;
		} break;
		case WEST: {
			return NORTH;
		} break;
		default: {
			puts("direction wtf?");
			return NORTH;
		} break;
	}
}

Direction cycle_left(Direction dir) {
	switch (dir) {
		case NORTH: {
			return WEST;
		} break;
		case WEST: {
			return SOUTH;
		} break;
		case SOUTH: {
			return EAST;
		} break;
		case EAST: {
			return NORTH;
		} break;
		default: {
			puts("direction wtf?");
			return NORTH;
		} break;
	}
}

int point_eq(Point a, Point b) {
	if (a.x == b.x && a.y == b.y && a.z == b.z) {
		return true;
	}
	return false;
}

Point new_point(u32 x, u32 y, u32 z) {
	Point p;
	p.x = x;
	p.y = y;
	p.z = z;

	return p;
}

void print_point(Point p) {
	printf("(%u, %u, %u)\n", p.x, p.y, p.z);
}

f32 lerp(f32 t, f32 start, f32 end) {
	return (1.0 - t) * start + (t * end);
}

#endif
