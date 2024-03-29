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
#include <libgen.h>
#include <sys/stat.h>

void usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <.pak file> <target directory>\n", prog_name);
}

int chkpath(char* path)
{
	size_t len = strlen(path);

	char* temp = (char*) malloc(len * sizeof(char) + 1);
	if (!temp) return -1;

	memset(temp, 0, len * sizeof(char) + 1);
	memcpy(temp, path, len * sizeof(char));

	char* ptr = temp;
	char* end = (char*)(temp + len * sizeof(char));

	unsigned long int count_backdir = 0;
	unsigned long int count_fwrddir = 0;
	char* last_sep = ptr;

	while (*ptr == '/') // skip over root directory
	{
		last_sep = ptr;
	        ptr++;
	}

	while (ptr < end)
	{
		if (*ptr == '/') // found a new subdir
		{
			*ptr = '\0';
			last_sep++;
			if (!strncmp(last_sep, "..", (size_t)(ptr - last_sep)))
			{
				count_backdir++;
			}
			else
			{
				if (strncmp(last_sep, ".", (size_t)(ptr - last_sep)))
				{
					count_fwrddir++;
				}
			}

			*ptr = '/';
			last_sep = ptr;
		}

		ptr++;
	}

	free(temp);

	return count_backdir > count_fwrddir ? -2 : 0;
}

/* The 'standard' mkdir doesn't allow recursive directory creation.
 * This one is capable of it :)
 *
 * TODO Non-Unix compatibility
 */
int mkdir2(char* path)
{
	size_t len = strlen(path);

	char* temp = (char*) malloc(len * sizeof(char) + 1);
	if (!temp) return -1;

	memset(temp, 0, len * sizeof(char) + 1);
	memcpy(temp, path, len * sizeof(char));
	if (temp[len - 1] == '/')
		temp[len - 1] = 0;

	char* ptr = temp;
	char* end = (char*)(temp + len * sizeof(char));

	while (*ptr++ == '/'); // skip over root directory

	while (ptr < end)
	{
		if (*ptr == '/') // found a new subdir
		{
			*ptr = 0;
			mkdir(temp, 0777);
			*ptr = '/';
		}
		ptr++;
	}

	mkdir(temp, 0777);

	free(temp);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		usage(argv[0]);
		return -1;
	}

	FILE* f = fopen(argv[1], "r");
	if (!f)
	{
		perror("Error opening file");
		return -1;
	}

	pakheader_t* hdr = (pakheader_t*) unpak_malloc(sizeof(pakheader_t));
	if (!hdr)
	{
		fprintf(stderr, "Error allocating memory...\n");
		fclose(f);
		return -1;
	}

	int ret = unpak_read_header(hdr, f);
	if (ret)
	{
		fprintf(stderr, "Error reading header... (ret = %i)\n", ret);
		unpak_free(hdr);
		fclose(f);
		return -1;
	}

	dirheader_t* dhdr = (dirheader_t*) unpak_malloc(sizeof(dirheader_t));
	if (!dhdr)
	{
		fprintf(stderr, "Error allocating memory...\n");
		unpak_free(hdr);
		fclose(f);
		return -1;
	}

	int i;
	int dhdr_num = unpak_dirheader_num(hdr, f);

	if (dhdr_num <= 0)
	{
		fprintf(stderr, "Error: Invalid number of dirheaders!\n");
		return -1;
	}

	printf("Number of files: %i\n", dhdr_num);

	for (i = 0; i < dhdr_num; i++)
	{
		int skipped = 0;
		memset(dhdr, 0, sizeof(dirheader_t));
		ret = unpak_read_dirheader(dhdr, hdr, i, f);
		if (ret)
		{
			fprintf(stderr, "Error reading dirheader... (ret = %i)\n", ret);
			goto skip_extracting;
		}

		printf("[%i/%i] Extracting %s... ", i + 1, dhdr_num, dhdr->name);
		fflush(stdout);

		int tpath_length = ((strlen(dhdr->name) + strlen(argv[2])) + 1);
		char* target_path = (char*) unpak_malloc(tpath_length);
		memset(target_path, 0, tpath_length);
		strcpy(target_path, argv[2]);
		strcat(target_path, dhdr->name);

		char* tpath_dirname = (char*) unpak_malloc(tpath_length);
		memset(tpath_dirname, 0, tpath_length);
		strcpy(tpath_dirname, target_path);
		tpath_dirname = dirname(tpath_dirname);

		if (chkpath(tpath_dirname))
		{
			fprintf(stderr, "\nIllegal path in dhdr, skipping this file...\n");
			goto skip_extracting;
		}

		if (mkdir2(tpath_dirname))
		{
			perror("\nError creating output directory structure");
			goto skip_extracting;
		}

		FILE* f_out = fopen(target_path, "w");
		if (!f_out)
		{
			perror("\nError creating output file");
			fprintf(stderr, "\n");
			goto skip_extracting;
		}

		void* buf = unpak_malloc(dhdr->size);
		if (!buf)
		{
			fprintf(stderr, "\nError allocating buffer...\n");
			fclose(f_out);
			goto skip_extracting;
		}

		ret = unpak_read_file(dhdr, buf, f);
		if (ret)
		{
			fprintf(stderr, "\nError reading file... (ret = %i)\n", ret);
			unpak_free(buf);
			fclose(f_out);
			goto skip_extracting;
		}

		ret = fwrite(buf, dhdr->size, 1, f_out);
		if (ret == 0)
		{
			perror("\nError writing to output file");
			fprintf(stderr, "\n");
			unpak_free(buf);
			fclose(f_out);
			goto skip_extracting;
		}

		unpak_free(buf);
		fclose(f_out);

		goto extracting_ok;

skip_extracting:
		if (skipped) fprintf(stderr, "\nSkipped extracting...\n");
		skipped = 1;

extracting_ok:
		if (!skipped) printf("\t[OK]\n");

extracting_done:
		fflush(stderr);
		unpak_free(target_path);
		unpak_free(tpath_dirname);

	}

	unpak_free(dhdr);
	unpak_free(hdr);
	fclose(f);

	return 0;
}
