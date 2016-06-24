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

void usage(char* prog_name)
{
	printf("%s <.pak file> <target directory>\n", prog_name);
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
		printf("Error allocating memory...\n");
		fclose(f);
		return -1;
	}

	int ret = unpak_read_header(hdr, f);
	if (ret)
	{
		printf("Error reading header... (ret = %i)\n", ret);
		unpak_free(hdr);
		fclose(f);
		return -1;
	}

	dirheader_t* dhdr = (dirheader_t*) unpak_malloc(sizeof(dirheader_t));
	if (!dhdr)
	{
		printf("Error allocating memory...\n");
		unpak_free(hdr);
		fclose(f);
		return -1;
	}

	int i;
	int dhdr_num = unpak_dirheader_num(hdr, f);
	printf("Number of files: %i\n", dhdr_num);

	for (i = 0; i < dhdr_num; i++)
	{
		int skipped = 0;
		memset(dhdr, 0, sizeof(dirheader_t));
		ret = unpak_read_dirheader(dhdr, hdr, i, f);
		if (ret)
		{
			printf("Error reading dirheader... (ret = %i)\n", ret);
			goto skip_extracting;
		}

		printf("Extracting %s (%i of %i)... ", dhdr->name, i + 1, dhdr_num);

		int tpath_length = ((strlen(dhdr->name) + strlen(argv[2])) + 1);
		char* target_path = (char*) unpak_malloc(tpath_length);
		memset(target_path, 0, tpath_length);
		strcpy(target_path, argv[2]);
		strcat(target_path, dhdr->name);

		char* tpath_dirname = (char*) unpak_malloc(tpath_length);
		memset(tpath_dirname, 0, tpath_length);
		strcpy(tpath_dirname, target_path);
		tpath_dirname = dirname(tpath_dirname);

		/*printf("[DBG] target_path = %s ", target_path);
		printf("dirname = %s ", tpath_dirname);
		mkdir(tpath_dirname, S_IRUSR | S_IWUSR);
		printf("target_path = %s\n", target_path);*/
		
		/* TODO: Make folder creation more portable */
		char* cmd = (char*) unpak_malloc(tpath_length + sizeof(char) * 11);
		memset(cmd, 0, tpath_length + sizeof(char) * 11);
		snprintf(cmd, tpath_length + sizeof(char) * 9, "mkdir -p \"%s\"", tpath_dirname);
		ret = system(cmd);
		if (ret != 0) printf("\nFailed creating folders...\n");
		unpak_free(cmd);

		FILE* f_out = fopen(target_path, "w");
		if (!f_out)
		{
			perror("\nError creating output file");
			printf("\n");
			goto skip_extracting;
		}

		void* buf = unpak_malloc(dhdr->size);
		if (!buf)
		{
			printf("\nError allocating buffer...\n");
			fclose(f_out);
			goto skip_extracting;
		}

		ret = unpak_read_file(dhdr, buf, f);
		if (ret)
		{
			printf("\nError reading file... (ret = %i)\n", ret);
			unpak_free(buf);
			fclose(f_out);
			goto skip_extracting;
		}

		ret = fwrite(buf, dhdr->size, 1, f_out);
		if (ret == 0)
		{
			perror("\nError writing to output file");
			printf("\n");
			unpak_free(buf);
			fclose(f_out);
			goto skip_extracting;
		}

		unpak_free(buf);
		fclose(f_out);

		goto extracting_ok;

skip_extracting:
		if (skipped) printf("\nSkipped extracting...\n");
		skipped = 1;

extracting_ok:
		if (!skipped) printf("\t[OK]\n");

		unpak_free(target_path);
		unpak_free(tpath_dirname);

	}

	unpak_free(dhdr);
	unpak_free(hdr);
	fclose(f);

	return 0;
}
