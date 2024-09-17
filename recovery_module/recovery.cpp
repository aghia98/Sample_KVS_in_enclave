/*

	shamir.c -- Shamir's Secret Sharing

	Inspired by:

		http://en.wikipedia.org/wiki/Shamir's_Secret_Sharing#Javascript_example


	Compatible with:

		http://www.christophedavid.org/w/c/w.php/Calculators/ShamirSecretSharing

	Notes:

		* The secrets start with 'AABBCC'
		* 'AA' is the hex encoded share # (1 - 255)
		* 'BB' is the threshold # of shares, also in hex
		* 'CC' is fake for compatibility with the web implementation above (= 'AA')
		* Remaining characters are encoded 1 byte = 2 hex characters, plus
			'G0' = 256 (since we use 257 prime modulus)

	Limitations:

		* rand() needs to be seeded before use (see below)


	Copyright © 2015 Fletcher T. Penney. Licensed under the MIT License.

	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "recovery.h"




static int prime = 257;

char * strtok_rr(
	char * str,
	const char * delim,
	char ** nextp) {
	char * ret;

	if (str == NULL) {
		str = *nextp;
	}

	if (str == NULL) {
		return NULL;
	}

	str += strspn(str, delim);

	if (*str == '\0') {
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str) {
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}



/*
	Math stuff
*/

int * gcdD(int a, int b) {
	int * xyz = static_cast<int*>(malloc(sizeof(int) * 3));

	if (b == 0) {
		xyz[0] = a;
		xyz[1] = 1;
		xyz[2] = 0;
	} else {
		int n = floor(a / b);
		int c = a % b;
		int * r = gcdD(b, c);

		xyz[0] = r[0];
		xyz[1] = r[2];
		xyz[2] = r[1] - r[2] * n;

		free(r);
	}

	return xyz;
}


/*
	More math stuff
*/

int modInverse(int k) {
	k = k % prime;

	int r;
	int * xyz;

	if (k < 0) {
		xyz = gcdD(prime, -k);
		r = -xyz[2];
	} else {
		xyz = gcdD(prime, k);
		r = xyz[2];
	}

	free(xyz);

	return (prime + r) % prime;
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c) {
	a = a - b;
	a = a - c;
	a = a ^ (c >> 13);
	b = b - c;
	b = b - a;
	b = b ^ (a << 8);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 13);
	a = a - b;
	a = a - c;
	a = a ^ (c >> 12);
	b = b - c;
	b = b - a;
	b = b ^ (a << 16);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 5);
	a = a - b;
	a = a - c;
	a = a ^ (c >> 3);
	b = b - c;
	b = b - a;
	b = b ^ (a << 10);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 15);
	return c;
}

void seed_random(void) {
	unsigned long seed = mix(clock(), time(NULL), getpid());
	srand(seed);
}

int modular_exponentiation(int base, int exp, int mod) {
	if (exp == 0) {
		return 1;
	} else if (exp % 2 == 0) {
		int mysqrt = modular_exponentiation(base, exp / 2, mod);
		return (mysqrt * mysqrt) % mod;
	} else {
		return (base * modular_exponentiation(base, exp - 1, mod)) % mod;
	}
}

void gener_R(int t, int s_new, int* coef){ //gener a polynomial R d(R)=t where R(s_new)=0
	seed_random();
	int i;
	
    (coef)[0] = rand() % (prime);  // Assign initial value

    // Generate random polynomial coefficients
    for (i = 1; i < t; ++i) {
        (coef)[i] = rand() % prime;
    }

    // Compute result on s_new
    int y = (coef)[0];
    for (i = 1; i < t; ++i) {
        int temp = modular_exponentiation(s_new, i, prime);
        y = (y + ((coef)[i] * temp % prime)) % prime;
    }

    // Adjust the first coefficient to satisfy some condition
	(coef)[0] -= y;
	while ((coef)[0]<0)
	{
		(coef)[0] = (coef)[0] + prime;
	}
	
	(coef)[0] = (coef)[0] % prime;

	/*for (i = 0; i < t; i++) {
		printf("%d\n",coef[i]);
	}*/

	//free(coef);

}

int R(int* coef ,int t, int node_id) {
	
	int y = coef[0];
	for (int i = 1; i < t; ++i) {
			int temp = modular_exponentiation(node_id, i, prime);

			y = (y + (coef[i] * temp % prime)) % prime;
		}
	y = (y + prime) % prime;

	return y;
}

void add_shares(const char* hex_share, int R, int* result_array) {
    int len = strlen(hex_share);  // Length of the hex string
	//result_array = (int*)malloc((len/2) * sizeof(int));
	

    // Iterate through the string 2 characters at a time
    for (int i = 0; i < len; i += 2) {
        char hex_pair[3];          // Temporary storage for 2 characters + null terminator
        strncpy(hex_pair, hex_share + i, 2);  // Copy 2 characters
        hex_pair[2] = '\0';         // Null-terminate the string

        // Convert hex string (hex_pair) to an integer
		int hex_value;
		if(strcmp(hex_pair,"G0")==0){
			hex_value = 256;
		}else{
			hex_value = (int)strtol(hex_pair, NULL, 16);
		}
        
        // Add R and perform modulo prime
        int result = (hex_value + R) % prime;
		while(result<0){
			result = result + prime;
		}
		result = result % prime;

        // Store the result in the array
        result_array[i / 2] = result;
    }
}

char hexDigit(int value) {
    if (value >= 0 && value <= 9) {
        return '0' + value;
    } else {
        return 'A' + (value - 10);
    }
}

void intToHex(int num, char *hex) {
    hex[0] = hexDigit((num >> 4) & 0xF);
    hex[1] = hexDigit(num & 0xF);
	if(num==256){
		hex[0]='G';
		hex[1]='0';
	}
    hex[2] = '\0'; // Null-terminate the string
}

char* convertToHex(int* input, int size_input, int x_share, int t) {
    char* result = (char*)malloc(sizeof(char) * size_input * 2 + 5);  // Allocate memory for the result string
    
    if (result) {
        char hex[3];
        char* ptr = result;
        
        intToHex(x_share, hex);
        *ptr++ = hex[0];
        *ptr++ = hex[1];
        
        intToHex(t, hex);
        *ptr++ = hex[0];
        *ptr++ = hex[1];
        
        *ptr++ = 'A';
		*ptr++ = 'A';
        
        for (int i = 0; i < size_input; i++) {
            intToHex(input[i], hex);
            *ptr++ = hex[0];
            *ptr++ = hex[1];
        }
        *ptr = '\0';  // Null-terminate the result string
    }
    
    return result;
}


/*int join_shares(int * xy_pairs, int n) {
	int secret = 0;
	long numerator;
	long denominator;
	long startposition;
	long nextposition;
	long value;
	int i;
	int j;

	// Pairwise calculations between all shares
	for (i = 0; i < n; ++i) {
		numerator = 1;
		denominator = 1;

		for (j = 0; j < n; ++j) {
			if (i != j) {
				startposition = xy_pairs[i * 2];		// x for share i
				nextposition = xy_pairs[j * 2];		// x for share j
				numerator = (numerator * (0-nextposition)) % prime;
				denominator = (denominator * (startposition - nextposition)) % prime;
			}
		}

		value = xy_pairs[i * 2 + 1];

		secret = (secret + (value * numerator * modInverse(denominator))) % prime;
	}


	secret = (secret + prime) % prime;
	//printf("%02X\n",secret);

	return secret;
}*/

int join_shares_at_point(int * xy_pairs, int n, int point) {
	int secret = 0;
	long numerator;
	long denominator;
	long startposition;
	long nextposition;
	long value;
	int i;
	int j;

	// Pairwise calculations between all shares
	for (i = 0; i < n; ++i) {
		numerator = 1;
		denominator = 1;

		for (j = 0; j < n; ++j) {
			if (i != j) {
				startposition = xy_pairs[i * 2];		// x for share i
				nextposition = xy_pairs[j * 2];		// x for share j
				numerator = (numerator * (point-nextposition)) % prime;
				denominator = (denominator * (startposition - nextposition)) % prime;
			}
		}

		value = xy_pairs[i * 2 + 1];

		secret = (secret + (value * numerator * modInverse(denominator))) % prime;
	}


	/* Sometimes we're getting negative numbers, and need to fix that */
	secret = (secret + prime) % prime;
	//printf("%02X\n",secret);

	return secret;
}



void free_string_shares(char ** shares, int n) {
	int i;

	for (i = 0; i < n; ++i) {
		free(shares[i]);
	}

	free(shares);
}


/*char * join_strings(char ** shares, int n) {
	

	if ((n == 0) || (shares == NULL) || (shares[0] == NULL)) {
		return NULL;
	}

	// `len` = number of hex pair values in shares
	int len = (strlen(shares[0]) - 6) / 2;

	char * result = static_cast<char*>(malloc(len + 1));
	char codon[3];
	codon[2] = '\0';	// Must terminate the string!

	int x[n];		// Integer value array
	int i;			// Counter
	int j;			// Counter

	// Determine x value for each share
	for (i = 0; i < n; ++i) {
		if (shares[i] == NULL) {
			free(result);
			return NULL;
		}

		codon[0] = shares[i][0];
		codon[1] = shares[i][1];

		x[i] = strtol(codon, NULL, 16);
	}

	// Iterate through characters and calculate original secret
	for (i = 0; i < len; ++i) {
		int * chunks = static_cast<int*>(malloc(sizeof(int) * n  * 2));

		// Collect all shares for character i
		for (j = 0; j < n; ++j) {
			// Store x value for share
			chunks[j * 2] = x[j];

			codon[0] = shares[j][6 + i * 2];
			codon[1] = shares[j][6 + i * 2 + 1];

			// Store y value for share
			if (memcmp(codon, "G0", 2) == 0) {
				chunks[j * 2 + 1] = 256;
			} else {
				chunks[j * 2 + 1] = strtol(codon, NULL, 16);
			}
		}

		//unsigned char letter = join_shares(chunks, n);
		unsigned char letter = join_shares(chunks, n);

		free(chunks);

		//sprintf(result + i, "%c", letter);
		result[i] = letter;
	}

	return result;
}*/

char * join_strings_at_point(char ** shares, int n, int point, int t) {

	if ((n == 0) || (shares == NULL) || (shares[0] == NULL)) {
		return NULL;
	}
	
	// `len` = number of hex pair values in shares
	int len_result = (strlen(shares[0]));

	char * result = static_cast<char*>(malloc(len_result + 1));
	sprintf(result, "%02X%02XAA", point,t);
	char codon[3];
	codon[2] = '\0';	// Must terminate the string!

	int x[n];		// Integer value array
	int i;			// Counter
	int j;			// Counter

	// Determine x value for each share

	
	for (i = 0; i < n; ++i) {
		if (shares[i] == NULL) {
			free(result);
			return NULL;
		}
		codon[0] = shares[i][0];
		codon[1] = shares[i][1];

		x[i] = strtol(codon, NULL, 16);
	}
	// Iterate through characters and calculate original secret
	for (i = 0; i < (len_result-6)/2; ++i) {
		
		int * chunks = (int*)malloc(sizeof(int) * n  * 2);

		// Collect all shares for character i
		for (j = 0; j < n; ++j) {
			// Store x value for share
			chunks[j * 2] = x[j];

			codon[0] = shares[j][6 + i * 2];
			codon[1] = shares[j][6 + i * 2 + 1];

			// Store y value for share
			if (memcmp(codon, "G0", 2) == 0) {
				chunks[j * 2 + 1] = 256;
			} else {
				chunks[j * 2 + 1] = strtol(codon, NULL, 16);
			}
		}

		int letter = join_shares_at_point(chunks, n, point);
		free(chunks);
		

		if(letter==256){
			sprintf(result + 6+2*i, "%s", "G0");
		}else{
			sprintf(result + 6+2*i, "%02X", letter);
		}
	}
	

	return result;
}




/* Trim spaces at end of string */
void trim_trailing_whitespace(char * str) {
	unsigned long l;

	if (str == NULL) {
		return;
	}

	l = strlen(str);

	if (l < 1) {
		return;
	}

	while ( (l > 0) && (( str[l - 1] == ' ' ) ||
						( str[l - 1] == '\n' ) ||
						( str[l - 1] == '\r' ) ||
						( str[l - 1] == '\t' )) ) {
		str[l - 1] = '\0';
		l = strlen(str);
	}
}


/*char * extract_secret_from_share_strings(const char * string) {
	char ** shares = static_cast<char**>(malloc(sizeof(char *) * 20));

	char * share;
	char * saveptr = NULL;
	int i = 0;

	
	char * temp_string = strdup(string);

	
	share = strtok_rr(temp_string, "\n", &saveptr);

	shares[i] = strdup(share);
	trim_trailing_whitespace(shares[i]);

	while ( (share = strtok_rr(NULL, "\n", &saveptr))) {
		i++;

		shares[i] = strdup(share);

		trim_trailing_whitespace(shares[i]);

		if ((shares[i] != NULL) && (strlen(shares[i]) == 0)) {
		
			free(shares[i]);
			i--;
		}
	}

	i++;

	char * secret = join_strings(shares, i);
	//free(secret);

	free_string_shares(shares, i);
	free(temp_string);

	return secret;
}*/

char * recover_share_from_share_strings(const char * string, int point, int t) {
	char ** shares = static_cast<char**>(malloc(sizeof(char *) * 20));

	char * share;
	char * saveptr = NULL;
	int i = 0;

	/* strtok_rr modifies the string we are looking at, so make a temp copy */
	char * temp_string = strdup(string);

	/* Parse the string by line, remove trailing whitespace */
	share = strtok_rr(temp_string, "\n", &saveptr);

	shares[i] = strdup(share);
	trim_trailing_whitespace(shares[i]);

	while ( (share = strtok_rr(NULL, "\n", &saveptr))) {
		i++;

		shares[i] = strdup(share);

		trim_trailing_whitespace(shares[i]);

		if ((shares[i] != NULL) && (strlen(shares[i]) == 0)) {
			/* Ignore blank lines */
			free(shares[i]);
			i--;
		}
	}

	i++;
	
	char * secret = join_strings_at_point(shares, i, point, t);
	free_string_shares(shares, i);
	free(temp_string);

	return secret;
}


