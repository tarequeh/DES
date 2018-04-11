#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * des.h provides the following functions and constants:
 *
 * generate_key, generate_sub_keys, process_message, ENCRYPTION_MODE, DECRYPTION_MODE
 *
 */
#include "des.h"

// Declare file handlers
static FILE *key_file, *input_file, *output_file;

// Declare action parameters
#define ACTION_GENERATE_KEY "-g"
#define ACTION_ENCRYPT "-e"
#define ACTION_DECRYPT "-d"

// DES key is 8 bytes long
#define DES_KEY_SIZE 8

int main(int argc, char* argv[]) {
	clock_t start, finish;
	double time_taken;
	unsigned long file_size;
	unsigned short int padding;

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
		short int bytes_written, process_mode;
		unsigned long block_count = 0, number_of_blocks;
		unsigned char* data_block = (unsigned char*) malloc(8*sizeof(char));
		unsigned char* processed_block = (unsigned char*) malloc(8*sizeof(char));
		key_set* key_sets = (key_set*)malloc(17*sizeof(key_set));

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
		while(fread(data_block, 1, 8, input_file)) {
			block_count++;
			if (block_count == number_of_blocks) {
				if (process_mode == ENCRYPTION_MODE) {
					padding = 8 - file_size%8;
					if (padding < 8) { // Fill empty data block bytes with padding
						memset((data_block + 8 - padding), (unsigned char)padding, padding);
					}

					process_message(data_block, processed_block, key_sets, process_mode);
					bytes_written = fwrite(processed_block, 1, 8, output_file);

					if (padding == 8) { // Write an extra block for padding
						memset(data_block, (unsigned char)padding, 8);
						process_message(data_block, processed_block, key_sets, process_mode);
						bytes_written = fwrite(processed_block, 1, 8, output_file);
					}
				} else {
					process_message(data_block, processed_block, key_sets, process_mode);
					padding = processed_block[7];

					if (padding < 8) {
						bytes_written = fwrite(processed_block, 1, 8 - padding, output_file);
					}
				}
			} else {
				process_message(data_block, processed_block, key_sets, process_mode);
				bytes_written = fwrite(processed_block, 1, 8, output_file);
			}
			memset(data_block, 0, 8);
		}

		finish = clock();

		// Free up memory
		free(des_key);
		free(data_block);
		free(processed_block);
		fclose(input_file);
		fclose(output_file);

		// Provide feedback
		time_taken = (double)(finish - start)/(double)CLOCKS_PER_SEC;
		printf("Finished processing %s. Time taken: %lf seconds.\n", argv[3], time_taken);
		return 0;
	} else {
		printf("Invalid action: %s. First parameter must be [ -g | -e | -d ].", argv[1]);
		return 1;
	}

	return 0;
}
