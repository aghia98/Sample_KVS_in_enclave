#ifndef SHAMIRS_SECRET_SHARING_H
#define SHAMIRS_SECRET_SHARING_H


#ifdef TEST
	#include "CuTest.h"
#endif

#include <vector>
#include "common.h"

using namespace std;



/// Given a secret, `n`, and `t`, create a list of shares (`\n` separated).
void seed_random(void);
char ** generate_share_strings(char * secret, int n, int t, vector<int> x_shares);


#endif
