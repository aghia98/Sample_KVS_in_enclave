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

#include "combine.h"

using namespace std;


//#define kBUFFERSIZE 4096	// How many bytes to read at a time

void copyString(string source, char* destination) {
    for (size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}


int main( int argc, char ** argv ) {
    // Read shares from stdin -- < shares.txt

    //DString * shares = stdin_buffer();
	string share1="0103AA0324EF82CF22252A4D157AA2A85A5AEC2E575DD11CB2D00C7EBBC19D55608BD2433B3D5C15BCA1B4581017F91652604DFE67107F434EFBD8F7B89D9B12C7A3630DB25E2DF82714A62B4915388E295F09017C10893D3E9AA6D6A0F0A88C9A9F58D560BDFFEE71FD0390D73B9381EBD1326DC3B8105F914D47C0F8E46FG016E3B3313187B7797D40E3";
	string share2="0303AAEEE8ED05799A3DD0DE02AD8EC2026B08419A97911AF4239C4F5DE5877FDD9813BC6983EE2E135705BF9CA1CA2243407CAD7356B6B0267280182A48F29C901B2E67CCD130DC261ADF0817G078F90F601F8179975E1AFC13B17B76E5ACAFEA34DA636D36769CAB2F7DA03EA503DC9EEF451A86BB93D06B48C3670B3A0E40C464C00A898B470898FB7C";
	string share3="0403AA15D74566C054A96B6D40B44B7FA2696E87FCE4C230DAEF960CADB422A84D60EB6694F9746A1FC2E8G08866EE685B2490CE79EDD7250A27879438C4195BD949E806AE1B3C0F4551BC220618CF4E125C7A754D5BE1ECD4586B9FE31D419506835A91736759BFB5A73F82161A37E6DFB06CACEA725B522D5A589E9E1C99D4936C8628E675787980EF76";

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
