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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**********************************
 * Constants
 **********************************/
#define MAGIC "PACK"


/**********************************
 * Structures
 **********************************/
typedef struct
{
	char id[4]; // Should be "PACK"
	int32_t offset;
	int32_t size;
} pakheader_t;

typedef struct
{
	char name[56];
	int32_t offset;
	int32_t size;
} dirheader_t;


/*********************************
 * Functions
 *********************************/
void* unpak_malloc(int size);

void unpak_free(void* ptr);

int unpak_read_header(pakheader_t* hdr, FILE* f);

int unpak_dirheader_num(pakheader_t* hdr, FILE* f);

int unpak_read_dirheader(dirheader_t* dhdr, pakheader_t* hdr, int index, FILE* f);

int unpak_read_file(dirheader_t* dhdr, void* target, FILE* f);
