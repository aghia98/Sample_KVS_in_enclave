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
#include <unistd.h>
#include <string>
#include <cstring>

//#include "../SS_no_gmp_module/ss-worker.h"
#include "recovery.h"

using namespace std;
//int prime=257;


//#define kBUFFERSIZE 4096	// How many bytes to read at a time

void copyString(string &source, char* destination) {
    for (size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; /// Append null character to terminate the string
	//strcpy(destination, source.c_str());
}


/*int main(int argc, char**argv){
	int t=3;
	int s_new=1;

	int* coef = static_cast<int*>(malloc(sizeof(int) * t)); // Allocate memory
	gener_R(t,s_new, coef);

	int y3 = R(coef, t, 3);
	int y5 = R(coef, t, 5);
	int y9 = R(coef, t, 9);

	int size = 63;
	int* result_array = (int*)malloc(size * sizeof(int));
	

	add_shares("4EFAA3D2758CAFEB5218364B035E606B099C05912BA8G06841EE9AD67CA18B4BB3FF8E695FDA50DE623FA1C6781C20C1782A6F5E40B8F928D79643CE01F895", y3, result_array);
	char* share3_new = convertToHex(result_array, size, 3, t);
	printf("%s\n",share3_new);

	add_shares("67D5D4D9F8D214B6F4151779E5CE49FE2E8D563C8632CBCBF04E43A40BC4D3BFDCC1E5B0AE63676A9EAE1325F1805EEEB6FECCAA3183ABF80A59C466A7AF71", y5, result_array);
	char*  share5_new = convertToHex(result_array, size, 5, t);
	printf("%s\n",share5_new);

	add_shares("C040ABE2B62E9463118BB1DE6C9F9623DDA235CD24773BFD7BA550C273AEB7FF95D93FCE0BCE84DFAAD37C9B1EB3E69281A530E1712BB300B94E4C157173E4", y9, result_array);
	char*  share9_new = convertToHex(result_array, size, 9, t);
	printf("%s\n",share9_new);

	free(result_array);
	free(coef);
}*/

int main( int argc, char ** argv ) {
    
	string share1="0303AA4EFAA3D2758CAFEB5218364B035E606B099C05912BA8G06841EE9AD67CA18B4BB3FF8E695FDA50DE623FA1C6781C20C1782A6F5E40B8F928D79643CE01F895";
	string share2="0503AA67D5D4D9F8D214B6F4151779E5CE49FE2E8D563C8632CBCBF04E43A40BC4D3BFDCC1E5B0AE63676A9EAE1325F1805EEEB6FECCAA3183ABF80A59C466A7AF71";
	string share3="0103AA4205EF1E308CDBD24F9A9D20643FA02EB1BC744E1E2D7D7F4DBF84885AB5B5A057C371A8A6C3891A02E95AA369DDE7ADG0ADA19DC4F3272765F845B531074C";


	string shares_string = share1+'\n'+share2+'\n'+share3+'\n';	

	char* combined_shares = static_cast<char*>(malloc((shares_string.length()+1)*sizeof(char)));
	copyString(shares_string, combined_shares); 


	//printf("%s\n",combined_shares);


	//char * secret = extract_secret_from_share_strings(combined_shares);
	//fprintf(stdout, "%s\n", secret);


	int point=4;
	int t=3;
	char* share = recover_share_from_share_strings(combined_shares, point, t);
	fprintf(stdout, "%s\n", share);

	free(combined_shares);
	//free(secret);
	free(share);
}
