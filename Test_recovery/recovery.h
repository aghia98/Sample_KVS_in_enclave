#ifndef SHAMIRS_SECRET_SHARING_H
#define SHAMIRS_SECRET_SHARING_H

//#include "strtok.h"

#ifdef TEST
	#include "CuTest.h"
#endif


//char * extract_secret_from_share_strings(const char * string);
char* recover_share_from_share_strings(const char* combined_shares, int point, int t);
void gener_R(int t, int s_new, int* coef);
int R(int* coef ,int t, int node_id);
void add_shares(const char* hex_share, int R, int* result_array);
char* convertToHex(int* input, int size_input, int x_share, int t);

#endif
