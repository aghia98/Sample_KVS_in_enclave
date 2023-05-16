#include "split.h"
#include <stdio.h>


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