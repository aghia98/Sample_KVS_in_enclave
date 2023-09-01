#ifndef SHAMIRS_SECRET_SHARING_H
#define SHAMIRS_SECRET_SHARING_H

#include "strtok.h"

#ifdef TEST
	#include "CuTest.h"
#endif


/// Given a list of shares (`\n` separated without leading whitespace), recreate the original secret.
char * extract_secret_from_share_strings(const char * string);
char* recover_share_from_string_shares(const char * string, int x_share, int t);
int* partial_recovery_of_share_from_one_share(const char * one_share, int x_share, int* x_shares, int x_shares_len);
char hexDigit(int value);
void intToHex(int num, char *hex);

//char* convertToHex(const char* input, int x_share, int t);

#endif
