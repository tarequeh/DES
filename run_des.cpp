#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <pthread.h>

/*
 * des.h provides the following functions and constants:
 *
 * generate_key, generate_sub_keys, process_message, ENCRYPTION_MODE, DECRYPTION_MODE
 *
 */
#include "des.h"

using namespace std;
// Declare file handlers
static FILE *key_file, *input_file, *output_file;

// Declare action parameters
#define ACTION_GENERATE_KEY "-g"
#define ACTION_ENCRYPT "-e"
#define ACTION_DECRYPT "-d"

// DES key is 8 bytes long
#define DES_KEY_SIZE 8

#define ucharmalloc (unsigned char*) malloc(8*sizeof(char))

#define MAX_POSIBLE_THREADS 16

short int process_mode;
vector<unsigned char*> output[MAX_POSIBLE_THREADS];
vector<unsigned char*> input[MAX_POSIBLE_THREADS];
unsigned long file_size;
key_set* key_sets = (key_set*)malloc(17*sizeof(key_set));
int nThread = 1;

void *converter(void* index) {
	clock_t start = clock();
	long i = (long) index;
	unsigned short int padding;
	vector<unsigned char*>::iterator it;
	for (it = input[i].begin(); it != input[i].end(); it++) {
		if (it + 1 == input[i].end() && i == nThread - 1) {
			padding = 8 - file_size%8;
			if (padding < 8) { // Fill empty data block bytes with padding
				memset((*it + 8 - padding), (unsigned char)padding, padding);
			}
		}
		output[i].push_back(ucharmalloc);
		process_message(*it, output[i].back(), key_sets, process_mode);
	}
	clock_t finish = clock();
	double time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;
	printf("thread[%ld] : %lf seconds\n", i, time_taken);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	clock_t start, finish;
	double time_taken;

	if (argc < 2) {
		printf("You must provide at least 1 parameter, where you specify the action.");
		return 1;
	}

	if (strcmp(argv[1], ACTION_GENERATE_KEY) == 0) { // Generate key file
		if (argc != 3) {
			printf("Invalid # of parameter specified. Usage: run_des -g keyfile.key");
			return 1;
		}

		key_file = fopen(argv[2], "wb");
		if (!key_file) {
			printf("Could not open file to write key.");
			return 1;
		}

		unsigned int iseed = (unsigned int)time(NULL);
		srand (iseed);

		short int bytes_written;
		unsigned char* des_key = (unsigned char*) malloc(8*sizeof(char));
		generate_key(des_key);
		bytes_written = fwrite(des_key, 1, DES_KEY_SIZE, key_file);
		if (bytes_written != DES_KEY_SIZE) {
			printf("Error writing key to output file.");
			fclose(key_file);
			free(des_key);
			return 1;
		}

		free(des_key);
		fclose(key_file);
	} else if ((strcmp(argv[1], ACTION_ENCRYPT) == 0) || (strcmp(argv[1], ACTION_DECRYPT) == 0)) { // Encrypt or decrypt
		if (argc != 5) {
			printf("Invalid # of parameters (%d) specified. Usage: run_des [-e|-d] keyfile.key input.file output.file", argc);
			return 1;
		}

		// Read key file
		key_file = fopen(argv[2], "rb");
		if (!key_file) {
			printf("Could not open key file to read key.");
			return 1;
		}

		short int bytes_read;
		unsigned char* des_key = (unsigned char*) malloc(8*sizeof(char));
		bytes_read = fread(des_key, sizeof(unsigned char), DES_KEY_SIZE, key_file);
		if (bytes_read != DES_KEY_SIZE) {
			printf("Key read from key file does nto have valid key size.");
			fclose(key_file);
			return 1;
		}
		fclose(key_file);

		// Open input file
		input_file = fopen(argv[3], "rb");
		if (!input_file) {
			printf("Could not open input file to read data.");
			return 1;
		}

		// Open output file
		output_file = fopen(argv[4], "wb");
		if (!output_file) {
			printf("Could not open output file to write data.");
			return 1;
		}

		// Generate DES key set
		short int bytes_written;
		unsigned long block_count = 0, number_of_blocks;
		unsigned char* data_block = (unsigned char*) malloc(8*sizeof(char));
		unsigned char* processed_block = (unsigned char*) malloc(8*sizeof(char));

		start = clock();
		generate_sub_keys(des_key, key_sets);
		finish = clock();
		time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;

		// Determine process mode
		if (strcmp(argv[1], ACTION_ENCRYPT) == 0) {
			process_mode = ENCRYPTION_MODE;
			printf("Encrypting..\n");
		} else {
			process_mode = DECRYPTION_MODE;
			printf("Decrypting..\n");
		}

		// Get number of blocks in the file
		fseek(input_file, 0L, SEEK_END);
		file_size = ftell(input_file);

		fseek(input_file, 0L, SEEK_SET);
		number_of_blocks = file_size/8 + ((file_size%8)?1:0);

		start = clock();

		// Start reading input file, process and write to output file
		pthread_t thread[nThread];
		unsigned long chunkSize = number_of_blocks / nThread;
		for (int i = 0; i < nThread; i++) {
			unsigned long start = chunkSize * i;
			unsigned long end = start + chunkSize;
			if (i == nThread - 1) {
				end = number_of_blocks;
			}
			for (block_count = start; block_count < end; block_count++) {
				input[i].push_back(ucharmalloc);
				fread(input[i].back(), 1, 8, input_file);
			}
		}

		clock_t start_convert = clock();
		// pthread_attr_t attr;
		// void *status;
		// pthread_attr_init(&attr);
		// pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		for(int i = 0; i < nThread; i++) {
			pthread_create(&thread[i], NULL, converter, (void*) i);
		}
		// pthread_attr_destroy(&attr);

		for(int i = 0; i < nThread; i++) {
			pthread_join(thread[i], NULL);
		}
		clock_t end_convert = clock();
		double time_convert_taken = (double)(end_convert - start_convert)/(double)CLOCKS_PER_SEC;
		printf("[%lf seconds]\n", time_convert_taken);

		vector<unsigned char*>::iterator it;
		for (int i = 0; i < nThread; i++) {
			for (it = output[i].begin(); it != output[i].end(); ++it) {
				bytes_written = fwrite(*it, 1, 8, output_file);
				delete *it;
			}
		}
		for (int i = 0; i < nThread; i++) {
			for (it = input[i].begin(); it != input[i].end(); ++it) {
				delete *it;
			}
			input[i].clear();
			output[i].clear();
		}

		// time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;
		// printf("Finished processing %s. Time taken: %lf seconds.\n", argv[3], time_taken);

		// Free up memory
		free(des_key);
		free(data_block);
		free(processed_block);
		fclose(input_file);
		fclose(output_file);

		// Provide feedback
		finish = clock();
		time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;
		printf("Finished processing %s. Time taken: %lf seconds.\n", argv[3], time_taken);
		pthread_exit(NULL);
		return 0;
	} else {
		printf("Invalid action: %s. First parameter must be [ -g | -e | -d ].", argv[1]);
		return 1;
	}
	return 0;
}
