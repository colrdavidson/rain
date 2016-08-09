#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

typedef struct DynArray {
	u32 size;
	void *arr[0];
} DynArray;

DynArray *new_dyn_array(u32 size) {
	DynArray *d = malloc(sizeof(DynArray) + (size * sizeof(void *)));
	d->size = size;

	return d;
}

#endif
