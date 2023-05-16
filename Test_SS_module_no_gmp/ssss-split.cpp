#include "split.h"
#include <stdio.h>
#include <iostream>

using namespace std;


int main( int argc, char ** argv ) {

	seed_random();
	if (argc == 3) { //split
		// Read secret from stdin -- n t < secrets.txt
		char ** shares;

		char secret[200];

		int n = atoi(argv[1]);

		int t = atoi(argv[2]);

		cin.sync_with_stdio(false);
  		if (cin.rdbuf()->in_avail() != 0) {
			string line;
			int secret_num = 1;
			while (cin.getline(secret, sizeof(secret))) { //read secrets one by one
				shares = generate_share_strings(secret, n, t);
				
				for(int i=0; i<n; i++) //display shares
					cout << shares[i] << endl; 
				free_string_shares(shares, n);
			}
			
  		}
		//****************************************************************************************

		
    }
}