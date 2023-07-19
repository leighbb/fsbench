/*
 * Title:		Agon MOS Filesystem benchmark
 * Author:		Leigh Brown
 * Created:		08/06/2023
 * Last Updated:	08/06/2023
 * 
 * Modinfo:
 */

#include <ez80.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ERRNO.H>

#include "mos-interface.h"

#define NUM_FILES		256
#define TEST_BLOCK_SIZE 	4096
#define TEST_FILE_SIZE		(1024 * 1024)
#define RANDOM_READ_COUNT	256
#define RANDOM_WRITE_COUNT	256

int errno; // needed by standard library

static const char *create_fn = "CREATE.TXT";
static const char *rename_fn = "RENAME.TXT";
static const char *test_fn = "TEST.DAT";
static const char crlf[2] = { '\r', '\n' };
static char buffer[TEST_BLOCK_SIZE];

void init_seed(void)
{
	DWORD t_start = getsysvar_time();
	UINT24 seed = (UINT24) t_start % UINT_MAX;
	srand(seed);
}

static const char b36tbl[37] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void gen_fname(char *buf, char suffix, int index)
{
        unsigned int rnd;
        unsigned int rem;
	char i;

        rnd = (rand() << 9) | (rand() % 511);
        for (i = 0; i < 4; ++i)
        {
                rem = rnd % 36;
                rnd = rnd / 36;
                *buf++ = b36tbl[rem];
        }
        rnd = (rand() << 9) | (rand() % 511);
        for (i = 0; i < 3; ++i)
        {
                rem = rnd % 36;
                rnd = rnd / 36;
                *buf++ = b36tbl[rem];
        }
	*buf++ = suffix;
	*buf++ = '.';
	sprintf(buf, "%03d", index % 1000);
}

int create_create_files(void)
{
	UINT8 file;
	char fname[12];
	int i, wrote;

	file = mos_fopen(create_fn, fa_write | fa_create_always);
	if (!file) {
		printf("Error opening '%s' for reading\r\n", fname);
		return -1;
	}

	for (i = 0; i < NUM_FILES; ++i) {
		gen_fname(fname, 'C', i);
		wrote = mos_fwrite(file, fname, sizeof(fname));
		if (wrote != sizeof(fname)) {
			printf("Error writing to '%s'\r\n", create_fn);
			mos_fclose(file);
			return -1;
		}
		wrote = mos_fwrite(file, crlf, sizeof(crlf));
		if (wrote != sizeof(crlf)) {
			printf("Error writing to '%s'\r\n", create_fn);
			mos_fclose(file);
			return -1;
		}
	}
	mos_fclose(file);

	return 0;
}

int create_rename_files(void)
{
	UINT8 file;
	char fname[12];
	int i, wrote;

	file = mos_fopen(rename_fn, fa_write | fa_create_always);
	if (!file) {
		printf("Error opening '%s' for reading\r\n", fname);
		return -1;
	}

	for (i = 0; i < NUM_FILES; ++i) {
		gen_fname(fname, 'R', i);
		wrote = mos_fwrite(file, fname, sizeof(fname));
		if (wrote != sizeof(fname)) {
			printf("Error writing to '%s'\r\n", rename_fn);
			mos_fclose(file);
			return -1;
		}
		wrote = mos_fwrite(file, crlf, sizeof(crlf));
		if (wrote != sizeof(crlf)) {
			printf("Error writing to '%s'\r\n", rename_fn);
			mos_fclose(file);
			return -1;
		}
	}
	mos_fclose(file);

	return 0;
}

int test1_creates(void)
{
	UINT8 file1, file2;
	int br;
	char fname[14];

	file1 = mos_fopen(create_fn, fa_read);
	if (!file1) {
		printf("Error opening '%s' for reading\r\n", create_fn);
		return 1;
	}

	while ((br = mos_fread(file1, fname, 14)) > 0) {
		fname[12] = '\0';
		file2 = mos_fopen(fname, fa_write|fa_create_always);
		if (!file2) {
			printf("Unable to create '%s'\r\n", fname);
			return -1;
		}
		mos_fclose(file2);
	}

	mos_fclose(file1);

	return 0;
}

int test2_renames(void)
{
	UINT8 file1, file2;
	int br, count;
	char fname1[14], fname2[14];

	file1 = mos_fopen(create_fn, fa_read);
	if (!file1) {
		printf("Error opening '%s' for reading\r\n", create_fn);
		return 1;
	}
	file2 = mos_fopen(rename_fn, fa_read);
	if (!file1) {
		printf("Error opening '%s' for reading\r\n", create_fn);
		mos_fclose(file1);
		return 1;
	}

	count = 0;
	while (1) {
		br = mos_fread(file1, fname1, 14);
		if (br != 14)
			break;
		fname1[12] = '\0';
		br = mos_fread(file2, fname2, 14);
		if (br != 14)
			break;
		fname2[12] = '\0';

		if (mos_ren(fname1, fname2) != 0) {
			printf("Rename of '%s' to '%s' failed\r\n", fname1, fname2);
			return -1;
		}
		++count;
	}

	mos_fclose(file1);
	mos_fclose(file2);

	return 0;
}

int test3_deletes(void)
{
	UINT8 file1;
	int br;
	char fname[14];

	file1 = mos_fopen(rename_fn, fa_read);
	if (!file1) {
		printf("Error opening '%s' for reading\r\n", rename_fn);
		return 1;
	}

	while ((br = mos_fread(file1, fname, 14)) > 0) {
		fname[12] = '\0';
		if (mos_del(fname) != 0) {
			printf("Unable to delete '%s'\r\n", fname);
			mos_fclose(file1);
			return 0;
		}
	}

	mos_fclose(file1);

	return 0;
}

int test4_create_write_file(void)
{
	UINT8 file;
	UINT24 bw;
	int size;
	char fname[14];

	file = mos_fopen(test_fn, fa_write|fa_create_always);
	if (!file) {
		printf("Error creating '%s' for writing\r\n", test_fn);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	size = TEST_FILE_SIZE;
	while (size > 0) {
		bw = mos_fwrite(file, buffer, sizeof(buffer));
		if (bw == 0) {
			printf("Error writing to '%s'\r\n", test_fn);
			mos_fclose(file);
		}
		size -= bw;
	}
	mos_fclose(file);

	return 0;
}

int test5_seq_read_file(void)
{
	UINT8 file;
	UINT24 bw;
	int size;
	char fname[14];

	file = mos_fopen(test_fn, fa_read);
	if (!file) {
		printf("Error creating '%s' for read\r\n", test_fn);
		return -1;
	}

	while ((bw = mos_fread(file, buffer, sizeof(buffer))) > 0)
			;

	mos_fclose(file);

	return 0;
}

int test6_seq_rewrite_file(void)
{
	UINT8 file;
	UINT24 bw;
	int size;
	char fname[14];

	file = mos_fopen(test_fn, fa_write|fa_open_existing);
	if (!file) {
		printf("Error creating '%s' for re-writing\r\n", test_fn);
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	size = TEST_FILE_SIZE;
	while (size > 0) {
		bw = mos_fwrite(file, buffer, sizeof(buffer));
		if (bw == 0) {
			printf("Error writing to '%s'\r\n", test_fn);
			mos_fclose(file);
		}
		size -= bw;
	}
	mos_fclose(file);

	return 0;
}

int test7_random_read_file(void)
{
	UINT8 file;
	DWORD loc;
	int i;
	char fname[14];

	file = mos_fopen(test_fn, fa_read);
	if (!file) {
		printf("Error creating '%s' for read\r\n", test_fn);
		return -1;
	}

	for (i = 0; i < RANDOM_READ_COUNT; ++i) {
		loc = (rand() % (TEST_FILE_SIZE / TEST_BLOCK_SIZE)) * TEST_BLOCK_SIZE;
		if (mos_flseek(file, loc) != 0) {
			printf("Error seeking in '%s'\r\n", test_fn);
			mos_fclose(file);
			return  -1;
		}
		if (mos_fread(file, buffer, sizeof(buffer)) != sizeof(buffer)) {
			printf("Error reading from '%s'\r\n", test_fn);
			mos_fclose(file);
			return -1;
		}
	}

	mos_fclose(file);

	return 0;
}

int test8_random_write_file(void)
{
	UINT8 file;
	DWORD loc;
	int i;
	char fname[14];

	file = mos_fopen(test_fn, fa_write);
	if (!file) {
		printf("Error creating '%s' for writing\r\n", test_fn);
		return -1;
	}

	for (i = 0; i < RANDOM_WRITE_COUNT; ++i) {
		loc = (rand() % (TEST_FILE_SIZE / TEST_BLOCK_SIZE)) * TEST_BLOCK_SIZE;
		if (mos_flseek(file, loc) != 0) {
			printf("Error seeking in '%s'\r\n", test_fn);
			mos_fclose(file);
			return  -1;
		}
		if (mos_fwrite(file, buffer, sizeof(buffer)) != sizeof(buffer)) {
			printf("Error writing to '%s'\r\n", test_fn);
			mos_fclose(file);
			return -1;
		}
	}

	mos_fclose(file);

	return 0;
}

int cleanup(void)
{
	mos_del(test_fn);
	mos_del(create_fn);
	mos_del(rename_fn);

	return 0;
}

int main(int argc, char * argv[]) {
	UINT8 file;
	UINT24 size;
	UINT24 br;
	DWORD t_start, t_end, t_elapsed;

	static const char *fname = "ONEMEG.BIN";
	static char buf[4096];

	printf("Initialising...");

	if (create_create_files() != 0)
		return -1;
	if (create_rename_files() != 0)
		return -1;
	printf("done.\r\n");

	printf("TEST 1 - Create files.......");
	t_start = getsysvar_time();
	if (test1_creates() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 2 - Rename files.......");
	t_start = getsysvar_time();
	if (test2_renames() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 3 - delete files.......");
	t_start = getsysvar_time();
	if (test3_deletes() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 4 - create and write...");
	t_start = getsysvar_time();
	if (test4_create_write_file() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 5 - sequential read....");
	t_start = getsysvar_time();
	if (test5_seq_read_file() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 6 - sequential write...");
	t_start = getsysvar_time();
	if (test6_seq_rewrite_file() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 7 - random read........");
	t_start = getsysvar_time();
	if (test7_random_read_file() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("TEST 8 - random write.......");
	t_start = getsysvar_time();
	if (test8_random_write_file() != 0)
		return -1;
	t_elapsed = getsysvar_time() - t_start;
	printf(" Took %d.%02d seconds\r\n", t_elapsed / 100, t_elapsed % 100);

	printf("All tests done!\n\r");
	cleanup();

	return 0;
}
