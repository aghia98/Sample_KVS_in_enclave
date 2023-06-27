#ifndef SHAMIRS_SECRET_SHARING_H
#define SHAMIRS_SECRET_SHARING_H

#include "strtok.h"
#include "common.h"

#ifdef TEST
	#include "CuTest.h"
#endif


/// Given a list of shares (`\n` separated without leading whitespace), recreate the original secret.
char * extract_secret_from_share_strings(const char * string);


#endif
