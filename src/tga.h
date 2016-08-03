#ifndef TGA_H
#define TGA_H

#include "common.h"

typedef struct Color {
	union {
		struct {
			u8 red;
			u8 green;
			u8 blue;
			u8 alpha;
		};
		u32 value;
	};
} Color;

typedef struct Image {
    u16 width;
    u16 height;
    u8 bytes_per_pixel;
    Color *data;
} Image;

typedef struct TGAHeader {
    u8 id_len;
    u8 colormap_t;
    u8 data_t;
    u16 colormap_origin;
    u8 colormap_depth;
    u16 x_origin;
    u16 y_origin;
    u16 width;
    u16 height;
    u8 bits_per_pixel;
    u8 img_desc;
} TGAHeader;

void print_color(u32 idx, Color *data) {
	printf("#%06X\n", data[idx].value);
}

Color to_color(u32 value) {
	Color c;
	c.value = value;
	return c;
}

void write_tga(const char *filename, Image *img) {
    FILE *out_file = fopen(filename, "wb");

    u8 dev_ref[4] = {0, 0, 0, 0};
    u8 ext_ref[4] = {0, 0, 0, 0};
    u8 footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};

    TGAHeader header;
    memset((void *)&header, 0, sizeof(header));
    header.bits_per_pixel = img->bytes_per_pixel << 3;
    header.width = img->width;
    header.height = img->height;
    header.data_t = 2;
    header.img_desc = 0x10;

    fwrite(&header, 1, sizeof(TGAHeader), out_file);
    fwrite((char *)img->data, 1, img->width * img->height * img->bytes_per_pixel, out_file);
    fwrite(dev_ref, 1, sizeof(dev_ref), out_file);
    fwrite(ext_ref, 1, sizeof(ext_ref), out_file);
    fwrite(footer, 1, sizeof(footer), out_file);

    fclose(out_file);
}

#endif
