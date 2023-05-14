#include "d_string.h"
#include "shamir.h"

#include <stdio.h>

#define kBUFFERSIZE 4096	// How many bytes to read at a time

DString * stdin_buffer() {
	/* Read from stdin and return a GString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, stdin)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);
	}

	fclose(stdin);

	return buffer;
}


int main( int argc, char ** argv ) {

	seed_random();
	if (argc == 3) { //split
		// Read secret from stdin -- n t < secrets.txt
		DString * secret = stdin_buffer();

		int n = atoi(argv[1]);

		int t = atoi(argv[2]);

		char ** shares = generate_share_strings(secret->str, n, t);

		for (int i = 0; i < n; ++i) printf("%s\n",shares[i]);

        free_string_shares(shares, n);
	    
		d_string_free(secret, true);
    }
}