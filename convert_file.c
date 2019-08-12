// SPDX-License-Identifier: GPL-3.0-or-later
/* gliden64_cache_extract, GLideN64 TexCache Extraction tool for debugging
 *
 * SPDX-FileCopyrightText: 2013-2015, Sven Eckelmann <sven@narfation.org>
 */

#include "gliden64_cache_extract.h"
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#pragma pack(push, 1)
struct bmp_header {
	int16_t identifier;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t dataofs;
	uint32_t headersize;
	uint32_t width;
	uint32_t height;
	int16_t planes;
	int16_t bitperpixel;
	uint32_t compression;
	uint32_t datasize;
	uint32_t hresolution;
	uint32_t vresolution;
	uint32_t colors;
	uint32_t importantcolors;
};

struct bmp_header_v5 {
	int16_t identifier;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t dataofs;
	uint32_t headersize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitperpixel;
	uint32_t compression;
	uint32_t datasize;
	uint32_t hresolution;
	uint32_t vresolution;
	uint32_t colors;
	uint32_t importantcolors;
	uint32_t redmask;
	uint32_t greenmask;
	uint32_t bluemask;
	uint32_t alphamask;
	uint32_t colorspace;
	uint32_t ciexyz_red_x;
	uint32_t ciexyz_red_y;
	uint32_t ciexyz_red_z;
	uint32_t ciexyz_green_x;
	uint32_t ciexyz_green_y;
	uint32_t ciexyz_green_z;
	uint32_t ciexyz_blue_x;
	uint32_t ciexyz_blue_y;
	uint32_t ciexyz_blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
	uint32_t intent;
	uint32_t profiledata_offset;
	uint32_t profiledata_size;
	uint32_t reserved2;
};
#pragma pack(pop)

static size_t image_content_length(const struct gliden64_file *file)
{
	size_t size;

	switch (file->format & ~GR_TEXFMT_GZ) {
	case GR_RGBA8:
		size = file->width * file->height * 4;
		break;
	case GR_RGB:
		size = file->width * file->height * 2;
		break;
	case GR_RGBA4:
		size = file->width * file->height * 2;
		break;
	case GR_RGB5_A1:
		size = file->width * file->height * 2;
		break;
	default:
		size = 0;
		fprintf(stderr, "Unsupported format %#"PRIx32"\n", file->format);
		break;
	}

	return size;
}

static int resize_image_bmp(struct gliden64_file *file)
{
	struct bmp_header *header;
	struct bmp_header_v5 *header_v5;
	uint32_t header_size;
	void *buf;
	uint8_t *imagedata;
	size_t line_size = file->width * 4;
	uint32_t i;

	if (file->format != GR_BGRA) {
		fprintf(stderr, "Unsupported texture format %#x for bmp export\n", file->format);
		return -EPERM;
	}

	if (globals.bitmapv5)
		header_size = (uint32_t)sizeof(*header_v5);
	else
		header_size = (uint32_t)sizeof(*header);

	if (file->size > (UINT32_MAX - header_size)) {
		fprintf(stderr, "Too large texture for bmp export\n");
		return -EPERM;
	}

	buf = malloc(file->size + header_size);
	if (!buf) {
		fprintf(stderr, "Memory for BMP file couldn't be allocated\n");
		return -ENOMEM;
	}

	if (globals.bitmapv5) {
		header_v5 = (struct bmp_header_v5 *)buf;
		memset(header_v5, 0, header_size);
		header_v5->identifier = htole16(0x4d42U);
		header_v5->filesize = htole32(file->size + header_size);
		header_v5->dataofs = htole32(header_size);
		header_v5->headersize = htole32(header_size - 14);
		header_v5->width = htole32(file->width);
		header_v5->height = htole32(file->height);
		header_v5->planes = htole16(1);
		header_v5->bitperpixel = htole16(32);
		header_v5->compression = htole32(3);
		header_v5->datasize = htole32(file->size);
		header_v5->hresolution = htole32(2835);
		header_v5->vresolution = htole32(2835);
		header_v5->colors = htole32(0);
		header_v5->importantcolors = htole32(0);
		header_v5->redmask = htole32(0x00ff0000U);
		header_v5->greenmask = htole32(0x0000ff00U);
		header_v5->bluemask = htole32(0x000000ffU);
		header_v5->alphamask = htole32(0xff000000U);
		header_v5->colorspace = htole32(0x73524742U);
		header_v5->ciexyz_red_x = htole32(0x00000000U);
		header_v5->ciexyz_red_y = htole32(0x00000000U);
		header_v5->ciexyz_red_z = htole32(0xfc1eb854U);
		header_v5->ciexyz_green_x = htole32(0x00000000U);
		header_v5->ciexyz_green_y = htole32(0x00000000U);
		header_v5->ciexyz_green_z = htole32(0xfc666666U);
		header_v5->ciexyz_blue_x = htole32(0x00000000U);
		header_v5->ciexyz_blue_y = htole32(0x00000000U);
		header_v5->ciexyz_blue_z = htole32(0xff28f5c4U);
		header_v5->gamma_red = htole32(0);
		header_v5->gamma_green = htole32(0);
		header_v5->intent = htole32(4);
		header_v5->profiledata_offset = htole32(4);
		header_v5->profiledata_size = htole32(4);
	} else {
		header = (struct bmp_header *)buf;
		memset(header, 0, header_size);
		header->identifier = htole16(0x4d42U);
		header->filesize = htole32(file->size + header_size);
		header->dataofs = htole32(header_size);
		header->headersize = htole32(header_size - 14);
		header->width = htole32(file->width);
		header->height = htole32(file->height);
		header->planes = htole16(1);
		header->bitperpixel = htole16(32);
		header->compression = htole32(0);
		header->datasize = htole32(file->size);
		header->hresolution = htole32(2835);
		header->vresolution = htole32(2835);
		header->colors = htole32(0);
		header->importantcolors = htole32(0);
	}

	imagedata = (uint8_t *)buf + header_size;
	for (i = 0; i < file->height; i++) {
		uint32_t target_line = i;
		uint32_t source_line = file->height - i - 1;
		uint8_t *target_pos = imagedata + target_line * line_size;
		uint8_t *source_pos = (uint8_t *)file->data;

		source_pos += source_line * line_size;

		memcpy(target_pos, source_pos, line_size);
	}
	free(file->data);
	file->data = buf;
	file->size += header_size;

	return 0;
}

static int normalize_image_r5g6b5(struct gliden64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	if (newsize > UINT32_MAX)
		return -EINVAL;

	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for R5G6B5 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		r = (raw & 0xf800U) >> 8;
		r |= r >> 5;
		g = (raw & 0x07e0U) >> 3;
		g |= g >> 6;
		b = (raw & 0x001fU) << 3;
		b |= b >> 5;
		p = (0xffU << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = (uint32_t)newsize;
	file->format = GR_BGRA;

	return 0;
}

static int normalize_image_r5g5b5a1(struct gliden64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	if (newsize > UINT32_MAX)
		return -EINVAL;

	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for R5G5B5A1 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		r = (raw & 0xf800U) >> 8;
		r |= r >> 5;
		g = (raw & 0x07c0U) >> 3;
		g |= g >> 5;
		b = (raw & 0x003eU) << 2;
		b |= b >> 5;
		a = (raw & 0x0001U);
		a *= 0xffU;
		p = (a << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = (uint32_t)newsize;
	file->format = GR_BGRA;

	return 0;
}

static int normalize_image_r4g4b4a4(struct gliden64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	if (newsize > UINT32_MAX)
		return -EINVAL;

	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for R4G4B4A4 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		r = (raw & 0xf000U) >> 4;
		r |= r >> 4;
		g = (raw & 0x0f00U) >> 4;
		g |= g >> 4;
		b = (raw & 0x00f0U);
		b |= b >> 4;
		a = (raw & 0x000fU) << 4;
		a |= a >> 4;
		p = (a << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = (uint32_t)newsize;
	file->format = GR_BGRA;

	return 0;
}

static int normalize_image_r8g8b8a8(struct gliden64_file *file)
{
	uint32_t *data, raw;
	size_t pixels, pos;
	uint32_t p, a, r, g, b;

	pixels = file->width * file->height;

	data = (uint32_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le32toh(data[pos]);
		a = (raw & 0xff000000U) >> 24;
		b = (raw & 0x00ff0000U) >> 16;
		g = (raw & 0x0000ff00U) >>  8;
		r = (raw & 0x000000ffU) >>  0;
		p = (a << 24) | (r << 16) | (g << 8) | b;
		data[pos] = htole32(p);
	}

	file->format = GR_BGRA;

	return 0;
}

static int resize_image_content(struct gliden64_file *file)
{
	int ret;

	switch (file->format) {
	case GR_RGB:
		ret = normalize_image_r5g6b5(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from RGB_565 to BGRA_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_RGB5_A1:
		ret = normalize_image_r5g5b5a1(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from RGBA_5551 to BGRA_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_RGBA4:
		ret = normalize_image_r4g4b4a4(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from RGBA_4444 to BGRA_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_RGBA8:
		ret = normalize_image_r8g8b8a8(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from RGBA_8888 to BGRA_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	default:
		fprintf(stderr, "Unsupported format %x\n", file->format);
		return -EPERM;
	}

	return 0;
}

int prepare_file(struct gliden64_file *file)
{
	size_t expected_size;
	void *buf;
	uLongf destLen;
	int ret;

	expected_size = image_content_length(file);
	if (expected_size > UINT32_MAX)
		return -EINVAL;

	if (file->format & GR_TEXFMT_GZ) {
		destLen = expected_size + 4096;
		buf = malloc(destLen);
		if (!buf) {
			fprintf(stderr, "Memory for uncompressing the file couldn't be allocated\n");
			return -ENOMEM;
		}

		ret = uncompress(buf, &destLen, file->data, file->size);
		if (ret != Z_OK) {
			free(buf);
			fprintf(stderr, "Failure during decompressing\n");
			return -EINVAL;
		}

		if (expected_size != destLen) {
			free(buf);
			fprintf(stderr, "Decompressed file has wrong filesize\n");
			return -EINVAL;
		}

		file->format &= ~GR_TEXFMT_GZ;
		free(file->data);
		file->data = buf;
		file->size = (uint32_t)expected_size;
	} else {
		if (expected_size != file->size) {
			fprintf(stderr, "Expected size of file is not the actual file size\n");
			return -EINVAL;
		}
	}

	ret = resize_image_content(file);
	if (ret < 0) {
		fprintf(stderr, "Failed to prepare image content\n");
		return ret;
	}

	return 0;
}
