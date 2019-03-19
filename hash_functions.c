# include <stdio.h>
# include <stdlib.h>
# include <string.h>
char *hash(FILE *fp) {
	// initialize variables for the counter and STDIN
	
	int scan_ret = 0;
	char c;
	char *file_hash = (char *)malloc(sizeof(char)*9);
	int i = 0;
	int block_size = 8;
	// while input is not EOF
	while (!feof(fp)){
		// Dereference the pointer and xor it with the input character
		scan_ret = fscanf(fp, "%c", &c);
		
		if (!feof(fp)){
		file_hash[i] = file_hash[i] ^ c;
		if (scan_ret == -1){
			perror("hash");		
		
		}
		// printf("%c", c);
		}
		
		// increment counter
		i += 1;
		// wrap around when the counter is equal to block_size by setting to 0
		if (i == block_size){
			i = 0;
		}
		
	}
	// show_hash(file_hash, 8);
	return file_hash;
	
}

void show_hash(char *hash_val, long block_size) {
    for(int i = 0; i < block_size; i++) {
        printf("%.2hhx ", hash_val[i]);
    }
    printf("\n");
}

int check_hash(const char *hash1, const char *hash2, long block_size) {
	// initialize counter
	int i = 0;
	// while the dereferenced hash elements are equal and counter is less than block size
	while(*(hash1 + i) == *(hash2 + i) && i < block_size){
		// increase counter
		i += 1;
	}
	
    return i;
}

