/*

	main.c -- Template main()

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

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../SS_no_gmp_module/ss-worker.h"
//#include "combine.h"

using namespace std;


//#define kBUFFERSIZE 4096	// How many bytes to read at a time

void copyString(string source, char* destination) {
    for (size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}


int main( int argc, char ** argv ) {
    

    //DString * shares = stdin_buffer();
	string share1="0303AA18E4F00FD039AB";
	string share2="0503AAA6A665B61F2AAC";
	string share3="0903AA227760ABAAFC2D";

	string shares_string = share1+'\n'+share2+'\n'+share3+'\n';	
	//const char* combined_shares = shares_string.c_str();
	char* combined_shares = static_cast<char*>(malloc((shares_string.length()+1)*sizeof(char)));
	copyString(shares_string, combined_shares); 

	printf("%s\n",combined_shares);

    char * secret = extract_secret_from_share_strings(combined_shares);

    fprintf(stdout, "%s\n", secret);

    free(secret);
    free(combined_shares);
}
