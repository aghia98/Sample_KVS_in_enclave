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
#include <time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "split.h"


static int prime = 257;


/*
	http://stackoverflow.com/questions/322938/recommended-way-to-initialize-srand

	http://www.concentric.net/~Ttwang/tech/inthash.htm

	Need a less predictable way to seed rand().
*/

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


/*
	from http://stackoverflow.com/questions/18730071/c-programming-to-calculate-using-modular-exponentiation

	Allows working with larger numbers (e.g. 255 shares, with a threshold of 200)
*/

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



/*
	split_number() -- Split a number into shares
	n = the number of shares
	t = threshold shares to recreate the number
*/

int * split_number(int number, int n, int t) {
	int * shares = static_cast<int*>(malloc(sizeof(int) * n));

	int * coef = static_cast<int*>(malloc(sizeof(int) * t));
	int x;
	int i;

	coef[0] = number;

	for (i = 1; i < t; ++i) {
		/* Generate random coefficients -- use arc4random if available */
#ifdef HAVE_ARC4RANDOM
		coef[i] = arc4random_uniform(prime);
#else
		coef[i] = rand() % (prime);
#endif
	}

	for (x = 0; x < n; ++x) {
		int y = coef[0];

		/* Calculate the shares */
		for (i = 1; i < t; ++i) {
			int temp = modular_exponentiation(x + 1, i, prime);

			y = (y + (coef[i] * temp % prime)) % prime;
		}

		/* Sometimes we're getting negative numbers, and need to fix that */
		y = (y + prime) % prime;

		shares[x] = y;
	}

	free(coef);

	return shares;
}

#ifdef TEST
void Test_split_number(CuTest * tc) {

	seed_random();

	int * test = split_number(1234, 50, 20);

	free(test);

	CuAssertIntEquals(tc, 0, 0);
}
#endif


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

#ifdef TEST
void Test_join_shares(CuTest * tc) {
	int n = 200;
	int t = 100;

	int shares[n * 2];

	int count = 255;	/* How many times should we test it? */
	int j;

	for (j = 0; j < count; ++j) {
		int * test = split_number(j, n, t);
		int i;

		for (i = 0; i < n; ++i) {
			shares[i * 2] = i + 1;
			shares[i * 2 + 1] = test[i];
		}

		/* Send all n shares */
		int result = join_shares(shares, n);

		free(test);

		CuAssertIntEquals(tc, j, result);
	}
}
#endif


/*
	split_string() -- Divide a string into shares
	return an array of pointers to strings;
*/

char ** split_string(char * secret, int n, int t) {
	int len = strlen(secret);

	char ** shares = static_cast<char**>(malloc (sizeof(char *) * n));
	int i;

	for (i = 0; i < n; ++i) {
		/* need two characters to encode each character */
		/* Need 4 character overhead for share # and quorum # */
		/* Additional 2 characters are for compatibility with:

			http://www.christophedavid.org/w/c/w.php/Calculators/ShamirSecretSharing
		*/
		shares[i] = (char *) malloc(2 * len + 6 + 1);

		sprintf(shares[i], "%02X%02XAA", (i + 1), t);
	}

	/* Now, handle the secret */

	for (i = 0; i < len; ++i) {
		int letter = secret[i]; // - '0';

		if (letter < 0) {
			letter = 256 + letter;
		}

		int * chunks = split_number(letter, n, t);
		int j;

		for (j = 0; j < n; ++j) {
			if (chunks[j] == 256) {
				sprintf(shares[j] + 6 + i * 2, "G0");	/* Fake code */
			} else {
				sprintf(shares[j] + 6 + i * 2, "%02X", chunks[j]);
			}
		}

		free(chunks);
	}

	return shares;
}


void free_string_shares(char ** shares, int n) {
	int i;

	for (i = 0; i < n; ++i) {
		free(shares[i]);
	}

	free(shares);
}


#ifdef TEST
void Test_split_string(CuTest * tc) {
	int n = 255;	/* Maximum n = 255 */
	int t = 254;	/* t <= n, we choose less than that so we have two tests */

	char * phrase = "This is a test of Bücher and Später.";

	int count = 10;
	int i;

	for (i = 0; i < count; ++i) {
		char ** result = split_string(phrase, n, t);

		/* Extract secret using first t shares */
		char * answer = join_strings(result, t);
		CuAssertStrEquals(tc, phrase, answer);
		free(answer);

		/* Extract secret using all shares */
		answer = join_strings(result, n);
		CuAssertStrEquals(tc, phrase, answer);
		free(answer);

		free_string_shares(result, n);
	}
}
#endif


char ** generate_share_strings(char * secret, int n, int t) {
	char ** result = split_string(secret, n, t);
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

