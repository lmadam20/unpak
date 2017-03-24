/* Copyright 2016 Leon Adam

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "pak.h"

void* unpak_malloc(int size)
{
	return malloc(size);
}

void unpak_free(void* ptr)
{
	if (ptr != NULL) free(ptr);
}

/*
 * Reads the file header into hdr.
 */
int unpak_read_header(pakheader_t* hdr, FILE* f)
{
	if (hdr == NULL) return -1;

	int ret = fread(hdr, sizeof(pakheader_t), 1, f);
	if (ret != 1) return -2;

	if (strncmp(hdr->id, MAGIC, 4)) return -3;
	return 0;
}

/*
 * Returns the number of dirheaders of hdr.
 */
int unpak_dirheader_num(pakheader_t* hdr, FILE* f)
{
	if (hdr == NULL) return -1;

	if ((hdr->size % sizeof(dirheader_t)) > 0) return -2;
	return hdr->size / sizeof(dirheader_t);
}

/*
 * Reads dhdr at index 'index' through hdr and f.
 */
int unpak_read_dirheader(dirheader_t* dhdr, pakheader_t* hdr, int index, FILE* f)
{
	if (dhdr == NULL || hdr == NULL) return -1;

	int ret = fseek(f, hdr->offset + index * sizeof(dirheader_t), SEEK_SET);
	if (ret != 0) return -2;

	ret = fread(dhdr, sizeof(dirheader_t), 1, f);
	if (ret != 1) return -2;

	return 0;
}

/*
 * Reads the file of dhdr into the buffer target.
 */
int unpak_read_file(dirheader_t* dhdr, void* target, FILE* f)
{
	if (target == NULL || dhdr == NULL) return -1;

	int ret = fseek(f, dhdr->offset, SEEK_SET);
	if (ret != 0) return -2;

	ret = fread(target, dhdr->size, 1, f);
	if (ret != 1) return -2;

	return 0;
}
