#ifndef PATH_H
#define PATH_H

#include "common.h"

typedef struct GridNode {
	u32 tile_id;
	struct GridNode **neighbors;
} GridNode;

typedef struct SearchNode {
	u8 visited;
	GridNode *tile;
	struct SearchNode *next;
} SearchNode;

#endif
