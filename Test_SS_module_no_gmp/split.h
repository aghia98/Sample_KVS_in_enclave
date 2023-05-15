#ifndef SHAMIRS_SECRET_SHARING_H
#define SHAMIRS_SECRET_SHARING_H


#ifdef TEST
	#include "CuTest.h"
#endif

#include "common.h"

/**

@file

@brief Simple library for providing SSS functionality.


*/


/// Given a secret, `n`, and `t`, create a list of shares (`\n` separated).
char ** generate_share_strings(char * secret, int n, int t);


#endif
