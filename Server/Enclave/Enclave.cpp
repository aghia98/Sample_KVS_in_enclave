/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include "sgx_trts.h"
#include "Enclave.h"
#include "Enclave_t.h"
#include <string>

//#include <string.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <map> // Secure Key-Value Store
#include <set>

#include "token.pb.h" 
//***********************************************
//***********************************************

using namespace std;
using namespace token;
// Declare myMap as a global variable
map<string, string> myMap;
//set<string> lost_keys_with_potential_last_share_owner_and_t_shares_owners;
int t;
int n;
int node_id;
int* poly_R;
map<int, int> stored_polynomial_shares;

map<string, vector<string>> blinded_shares;

string temp_token;
int prime = 257;
//map<int,string> node_id_to_token_temp;

/* used to eliminate `unused variable' warning */
#define UNUSED(val) (void)(val)

#define ULP 2

#define MAX_SIZE_VALUE 264
#define MAX_SIZE_SERIALIZED_TOKEN 764

/*char* convertCString(std::string str) {
    char* cstr = new char[str.length() + 1];  // +1 for null-terminator
    strcpy(cstr, str.c_str());
    return cstr;
}*/

char * enclave_strcpy(char * dest, const char * src) {
    if (dest == nullptr || src == nullptr) {
        return nullptr;  // Return nullptr if either input is null
    }

    char * original_dest = dest;  // Save the original pointer to return later

    // Copy each character from source to destination
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    // Null-terminate the destination string
    *dest = '\0';

    return original_dest;
}

// Custom strdup implementation
char * enclave_strdup(const char * string) {
    if (string == nullptr) {
        return nullptr;
    }

    // Allocate memory for the new string
    size_t len = 0;
    while (string[len] != '\0') {  // Get the length of the string manually
        len++;
    }
    len++;  // Add 1 for the null terminator

    char * temp_string = (char *) malloc(len);

    // Check if allocation succeeded
    if (temp_string != nullptr) {
        enclave_strcpy(temp_string, string);  // Copy the original string using the custom strcpy
    }

    return temp_string;
}

uint32_t enclave_rand() {
    uint32_t rand_num = 0;
    sgx_status_t status = sgx_read_rand((unsigned char*)&rand_num, sizeof(rand_num));

    if (status != SGX_SUCCESS) {
        // Handle error appropriately
        printf("Failed to generate random number!\n");
    }

    return rand_num;
}

char * strtok_rr(
	char * str,
	const char * delim,
	char ** nextp) {
	char * ret;

	if (str == NULL) {
		str = *nextp;
	}

	if (str == NULL) {
		return NULL;
	}

	str += strspn(str, delim);

	if (*str == '\0') {
		return NULL;
	}

	ret = str;

	str += strcspn(str, delim);

	if (*str) {
		*str++ = '\0';
	}

	*nextp = str;

	return ret;
}

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


void gener_R(int t, int s_new, int* coef){ //gener a polynomial R d(R)=t where R(s_new)=0
	//seed_random();
	int i;
	
    (coef)[0] = enclave_rand() % (prime);  // Assign initial value

    // Generate random polynomial coefficients
    for (i = 1; i < t; ++i) {
        (coef)[i] = enclave_rand() % prime;
    }

    // Compute result on s_new
    int y = (coef)[0];
    for (i = 1; i < t; ++i) {
        int temp = modular_exponentiation(s_new, i, prime);
        y = (y + ((coef)[i] * temp % prime)) % prime;
    }

    // Adjust the first coefficient to satisfy some condition
	(coef)[0] -= y;
	while ((coef)[0]<0)
	{
		(coef)[0] = (coef)[0] + prime;
	}
	
	(coef)[0] = (coef)[0] % prime;


	//free(coef);

}

int R(int* coef ,int t, int node_id) {
	
	int y = coef[0];
	for (int i = 1; i < t; ++i) {
			int temp = modular_exponentiation(node_id, i, prime);

			y = (y + (coef[i] * temp % prime)) % prime;
		}
	y = (y + prime) % prime;

	return y;
}

void add_shares(const char* hex_share, int R, int* result_array) {
    int len = strlen(hex_share);  // Length of the hex string
	//result_array = (int*)malloc((len/2) * sizeof(int));
	

    // Iterate through the string 2 characters at a time
    for (int i = 0; i < len; i += 2) {
        char hex_pair[3];          // Temporary storage for 2 characters + null terminator
        strncpy(hex_pair, hex_share + i, 2);  // Copy 2 characters
        hex_pair[2] = '\0';         // Null-terminate the string

        // Convert hex string (hex_pair) to an integer
		int hex_value;
		if(strcmp(hex_pair,"G0")==0){
			hex_value = 256;
		}else{
			hex_value = (int)strtol(hex_pair, NULL, 16);
		}
        
        // Add R and perform modulo prime
        int result = (hex_value + R) % prime;
		while(result<0){
			result = result + prime;
		}
		result = result % prime;

        // Store the result in the array
        result_array[i / 2] = result;
    }
}



char hexDigit(int value) {
    if (value >= 0 && value <= 9) {
        return '0' + value;
    } else {
        return 'A' + (value - 10);
    }
}

void intToHex(int num, char *hex) {
    hex[0] = hexDigit((num >> 4) & 0xF);
    hex[1] = hexDigit(num & 0xF);
	if(num==256){
		hex[0]='G';
		hex[1]='0';
	}
    hex[2] = '\0'; // Null-terminate the string
}

char* convertToHex(int* input, int size_input, int x_share, int t) {
    char* result = (char*)malloc(sizeof(char) * size_input * 2 + 5);  // Allocate memory for the result string
    
    if (result) {
        char hex[3];
        char* ptr = result;
        
        intToHex(x_share, hex);
        *ptr++ = hex[0];
        *ptr++ = hex[1];
        
        intToHex(t, hex);
        *ptr++ = hex[0];
        *ptr++ = hex[1];
        
        *ptr++ = 'A';
		*ptr++ = 'A';
        
        for (int i = 0; i < size_input; i++) {
            intToHex(input[i], hex);
            *ptr++ = hex[0];
            *ptr++ = hex[1];
        }
        *ptr = '\0';  // Null-terminate the result string
    }
    
    return result;
}

int join_shares_at_point(int * xy_pairs, int n, int point) {
	int secret = 0;
	long numerator;
	long denominator;
	long startposition;
	long nextposition;
	long value;
	int i;
	int j;

	// Pairwise calculations between all shares
	for (i = 0; i < n; ++i) {
		numerator = 1;
		denominator = 1;

		for (j = 0; j < n; ++j) {
			if (i != j) {
				startposition = xy_pairs[i * 2];		// x for share i
				nextposition = xy_pairs[j * 2];		// x for share j
				numerator = (numerator * (point-nextposition)) % prime;
				denominator = (denominator * (startposition - nextposition)) % prime;
			}
		}

		value = xy_pairs[i * 2 + 1];

		secret = (secret + (value * numerator * modInverse(denominator))) % prime;
	}


	secret = (secret + prime) % prime;
	//printf("%02X\n",secret);

	return secret;
}

void free_string_shares(char ** shares, int n) {
	int i;

	for (i = 0; i < n; ++i) {
		free(shares[i]);
	}

	free(shares);
}

char * join_strings_at_point(char ** shares, int n, int point, int t) {

	if ((n == 0) || (shares == NULL) || (shares[0] == NULL)) {
		return NULL;
	}
	
	// `len` = number of hex pair values in shares
	int len_result = (strlen(shares[0]));

	char * result = static_cast<char*>(malloc(len_result + 1));
	//sprintf(result, "%02X%02XAA", point,t);
    //I cannot use sprintf...
    char hex[3];
    intToHex(point, hex);
    result[0] = hex[0];
    result[1] = hex[1];
    intToHex(t, hex);
    result[2] = hex[0];
    result[3] = hex[1];
    result[4] = 'A';
    result[5] = 'A';

	char codon[3];
	codon[2] = '\0';	// Must terminate the string!

	int x[n];		// Integer value array
	int i;			// Counter
	int j;			// Counter

	// Determine x value for each share

	
	for (i = 0; i < n; ++i) {
		if (shares[i] == NULL) {
			free(result);
			return NULL;
		}
		codon[0] = shares[i][0];
		codon[1] = shares[i][1];

		x[i] = strtol(codon, NULL, 16);
	}
	// Iterate through characters and calculate original secret
	for (i = 0; i < (len_result-6)/2; ++i) {
		
		int * chunks = (int*)malloc(sizeof(int) * n  * 2);

		// Collect all shares for character i
		for (j = 0; j < n; ++j) {
			// Store x value for share
			chunks[j * 2] = x[j];

			codon[0] = shares[j][6 + i * 2];
			codon[1] = shares[j][6 + i * 2 + 1];

			// Store y value for share
			if (memcmp(codon, "G0", 2) == 0) {
				chunks[j * 2 + 1] = 256;
			} else {
				chunks[j * 2 + 1] = strtol(codon, NULL, 16);
			}
		}

		int letter = join_shares_at_point(chunks, n, point);
		free(chunks);
		
        intToHex(letter, hex);
        result[6+2*i] = hex[0];
        result[7+2*i] = hex[1];

		/*if(letter==256){
			sprintf(result + 6+2*i, "%s", "G0");
		}else{
			sprintf(result + 6+2*i, "%02X", letter);
		}*/
	}

    result[len_result] = '\0';
	

	return result;
}

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

int compute_sum_polynomial_shares(){
    int sum=0;
    for (auto it = stored_polynomial_shares.begin(); it != stored_polynomial_shares.end(); ++it) {
        sum += it->second;
    }

    return sum;
}

void ecall_get_blinded_share(char key[], char* val){
    string key_string(key);
    auto iterator = myMap.find(key_string);

    string share = iterator->second;
    string share_without_metadata = share.substr(6);
    const char* share_without_metadata_cstr = share_without_metadata.c_str();

    int size_result = strlen(share_without_metadata_cstr)/2;
    int* result_array = (int*)malloc(size_result * sizeof(int));
    add_shares(share_without_metadata_cstr, compute_sum_polynomial_shares(), result_array);
    char* blinded_share = convertToHex(result_array, size_result, node_id, t);

    strncpy(val, blinded_share, strlen(val));
    free(result_array);
    free(blinded_share);
}

char * recover_share_from_share_strings(const char * string, int point, int t) {
	char ** shares = static_cast<char**>(malloc(sizeof(char *) * 20));

	char * share;
	char * saveptr = NULL;
	int i = 0;

	/* strtok_rr modifies the string we are looking at, so make a temp copy */
	char * temp_string = enclave_strdup(string);

	/* Parse the string by line, remove trailing whitespace */
	share = strtok_rr(temp_string, "\n", &saveptr);

	shares[i] = enclave_strdup(share);
	trim_trailing_whitespace(shares[i]);

	while ( (share = strtok_rr(NULL, "\n", &saveptr))) {
		i++;

		shares[i] = enclave_strdup(share);

		trim_trailing_whitespace(shares[i]);

		if ((shares[i] != NULL) && (strlen(shares[i]) == 0)) {
			/* Ignore blank lines */
			free(shares[i]);
			i--;
		}
	}

	i++;
	
	char * secret = join_strings_at_point(shares, i, point, t);
	free_string_shares(shares, i);
	free(temp_string);

	return secret;
}


void copyString(std::string source, char* destination) {
    for (std::size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}

void ecall_store_blinded_share(char key[], char blinded_share[]){
    string key_string(key);
    string val_string(blinded_share);
    blinded_shares[key_string].push_back(val_string);

    if(blinded_shares[key_string].size() == t){
        string shares_string = "";
        for(string& blinded_shares: blinded_shares[key_string]){
            shares_string += blinded_shares+'\n';
        }
        char* combined_shares = static_cast<char*>(malloc((shares_string.length()+1)*sizeof(char)));
        copyString(shares_string, combined_shares); 
        char* share = recover_share_from_share_strings(combined_shares, node_id, t);
        
        string share_string(share);
        myMap[key_string] = share_string;
        
        free(combined_shares);
        free(share);
    }else{
        if(blinded_shares[key_string].size() == n){
            blinded_shares.erase(key_string);
        }
    }
    

    
}

void ecall_process_end_of_recovery(){
    blinded_shares.clear();
    stored_polynomial_shares.clear();
    free(poly_R);
}


int printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return 0;
}

void ecall_send_params_to_enclave(int* node, int* t_, int *n_){
    node_id = (*node);
    t = (*t_);
    n = (*n_);

    printf("t: %d and n: %d and node_id: %d\n",t,n,node_id);
}

void ecall_put(char key[], char val[]){
    string key_string(key);
    string val_string(val);
    myMap[key_string]=val_string;
    //printf("%s\n", myMap[key_string].c_str());
}

void ecall_get(char key[], char* val){
  string key_string(key);
  
  auto iterator = myMap.find(key_string);
  string source;
  if(iterator != myMap.end()){
    source = iterator->second;
    strncpy(val, source.c_str(), strlen(val));
  }else{
    strncpy(val, "NOT_FOUND", strlen(val));
  } 
}




void ecall_delete(char key[]){
  string key_string(key);
  myMap.erase(key_string);
  
  //printf("%s deleted \n", key);
}

//********************************************************************************************************************************
bool isDigit(char c) {
    return (c >= '0' && c <= '9');
} 

int extractNumber(string& input) {
    string numberStr;
    bool foundNumber = false;

    for (char c : input) {
        if (isDigit(c)) {
            numberStr += c;
            foundNumber = true;
        } else if (foundNumber) {
            // Break loop if a digit was found and a non-digit character follows
            break;
        }
    }

    if (foundNumber) {
        int number = std::stoi(numberStr);
        return number;
    } else {
        return -1; // Indicate no number found
    }
} 

uint32_t jenkinsHash(string& server_id_with_string) {
    const uint8_t* data = (const uint8_t*) server_id_with_string.c_str();
    uint32_t hash = 0;

    for (size_t i = 0; i < server_id_with_string.size(); ++i) {
        hash += data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

vector<pair<string, uint32_t>> sortMapByValue(const map<string, uint32_t>& inputMap) {
    // Create a vector of pairs from the map
    vector<pair<string, uint32_t>> pairs(inputMap.begin(), inputMap.end());

    // Sort the vector based on the value member using a lambda function
    sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    return pairs;
}

vector<pair<string, uint32_t>> order_HRW(const std::vector<string>& ids, string& key) {
    //vector<int> sortedArr = ids;  // Create a copy of the input array
    
    map<string, uint32_t> ordered_ids_to_hash;
    
    // Sort the array in ascending order
    for(string id:ids){
        string to_hash = id+key;
        ordered_ids_to_hash[id]=jenkinsHash(to_hash);

    }
    vector<pair<string, uint32_t>> sortedPairs = sortMapByValue(ordered_ids_to_hash);
    
    return sortedPairs;
}


vector<string> convert_ids_to_strings_with_id(vector<int> ids, string word){
    vector<string> result;
    for(int id: ids){
        result.push_back(word+to_string(id));
    }  

    return result;
 }

vector<int> convert_strings_with_id_to_ids(vector<string> strings_with_id){
    vector<int> result;
    for(string string_with_id: strings_with_id){
        result.push_back(extractNumber(string_with_id));
    }  

    return result;
 } 

void ecall_share_lost_keys(int* node_id, int* s_up_ids_array, unsigned cnt, char* lost_keys_with_potential_last_share_owner) {
  //******************************7

    vector<string> strings_with_id_of_N_active;
    vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;
    vector<int> s_up_ids_vector;
    string word = "server";
    string keys="";
    int j=0;
    //convert int* to vector for s_up_ids_array
    for (int i = 0; i < cnt; ++i) {
        j++;
        s_up_ids_vector.push_back(s_up_ids_array[i]);
    }
    strings_with_id_of_N_active = convert_ids_to_strings_with_id(s_up_ids_vector, word);
  
    for (auto it = myMap.begin(); it != myMap.end(); it++) {
        string  k = it->first; //Secret_n
        ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,k); //Order according to HRW
        
        string spawned_node_id = word+to_string(*node_id); //serverX
        string spawned_node_to_hash = spawned_node_id+k; //ServerX+k
        //strncpy(lost_keys, spawned_node_to_hash.c_str(), strlen(lost_keys));
        uint32_t spawned_node_hash_wrt_k = jenkinsHash(spawned_node_to_hash);
        uint32_t n_th_node_hash;
        string potential_last_share_owner_id;

        if(cnt<n){
            n_th_node_hash = 0;
            potential_last_share_owner_id = "null"; // number of active nodes is smaller than n
        }else{
            n_th_node_hash = ordered_strings_with_id_to_hash[n-1].second;
            potential_last_share_owner_id = to_string(extractNumber(ordered_strings_with_id_to_hash[n-1].first));
        }

        if(spawned_node_hash_wrt_k > n_th_node_hash){ //spawned_node is a neighbour for k
            //gather at least t share owners
            string t_shares_owners = "";


            for(int i=0; i<t; i++){
                t_shares_owners += to_string(extractNumber(ordered_strings_with_id_to_hash[i].first));
                if(i<t-1){
                    t_shares_owners +=",";
                }
            }
            keys += k+"|"+potential_last_share_owner_id+"|"+t_shares_owners+"\n";
            
        }
    }
    
    strncpy(lost_keys_with_potential_last_share_owner, keys.c_str(), strlen(lost_keys_with_potential_last_share_owner)); 
    //printf(lost_keys);
}

/*void ecall_add_lost_keys(char keys_with_last_share_owner[]){

    if(strcmp(keys_with_last_share_owner,"null\n")!=0){
        string line;
        
        while (*keys_with_last_share_owner != '\0') {
            if (*keys_with_last_share_owner == '\n') {
                if (!line.empty()) {
                    lost_keys_with_potential_last_share_owner_and_t_shares_owners.insert(line);
                    line.clear();
                }
            } else {
                line += *keys_with_last_share_owner;
            }
            keys_with_last_share_owner++;
        }
    }    
}*/
//*****************************************************************************************
vector<string> splitString(const string& input, char delimiter) {
    vector<string> result;
    size_t start = 0;
    size_t end = input.find(delimiter);
    
    while (end != string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + 1;
        end = input.find(delimiter, start);
    }
    
    result.push_back(input.substr(start));
    return result;
}



Token init_token(string& key, vector<int> path, int len_cumul){
    Token token_message;

    // Fill in the fields
    token_message.set_initiator_id(node_id);  // Replace 123 with your desired value
    token_message.set_key(key);    // Replace "your_key" with your desired value

    //printf("len_cumul: %d\n", len_cumul);
    for(int i=0; i< len_cumul; i++){
        token_message.add_cumul(1);
    }
    

    token_message.set_passes(0);          // Set the 'passes' field to your desired value

    for(const int s_up_id: path){
        token_message.add_path(s_up_id);
    }

    //std::string serialized_message;
    //token_message.SerializeToString(&serialized_message);
    //local_print_token(serialized_message.c_str());

    //ocall_print_token(serialized_message.c_str());

    return token_message;
}

void distributed_polynomial_interpolation(Token token){
    /*string serialized_token;

    token.SerializeToString(&serialized_token);
    //ocall_print_token(serialized_token.c_str());

    if(node_id!=token.initiator_id()){
        //TBD: compute the partial sum
        //get value
        char share[MAX_SIZE_VALUE];
        memset(share, 'A', MAX_SIZE_VALUE-1);
        char token_key_char[100];
        copyString(token.key(), token_key_char); 
        ecall_get(token_key_char, share);
        
        //get path for x_shares
        int* x_shares = static_cast<int*>(malloc(token.path().size()*sizeof(int)));
        for (int i = 0; i < token.path().size(); ++i) {
            x_shares[i] = token.path(i);
        }

        //printf("share: %s\n",share);

        int* sub_share = partial_recovery_of_share_from_one_share(share, token.initiator_id() , x_shares, token.path().size());
        
        int len_sub_share = (strlen(share) - 6) / 2;

        for(int i=0; i< len_sub_share; i++){
            token.set_cumul(i, token.cumul(i)+sub_share[i]);
        }

        free(sub_share);
        free(x_shares);

        token.set_passes(token.passes()+1);
        //TBD: send token to next node if it exists, else stores it in a temporal Token variable
        if(token.passes() < token.path().size()){
            int next_node_id = token.path(token.passes());
            token.SerializeToString(&serialized_token);
            //printf("afterrrrrrrrrrrrrr\n");
            //ocall_print_token(serialized_token.c_str());
            ocall_send_token(serialized_token.c_str(), &next_node_id);
        }else{ //store the token
            
            //printf("heeeeeeeeeeeeeeereeee ready to store\n");
            
            token.SerializeToString(&serialized_token);
            temp_token = serialized_token;
            //ocall_print_token(serialized_token.c_str());
        }
    }else{
        if(token.passes()==0){
            int next_node_id = token.path(0);
            ocall_send_token(serialized_token.c_str(), &next_node_id);
        }else{
            //TBD: store token result
        }
    }*/
}

/*void store_share(const char *serialized_token){
    Token token;
    
    token.ParseFromString(serialized_token);
    vector<int> got_share;
    int value;
    

    //printf("size of cumul: %d\n", token.cumul_size());
    for(int i = 0; i < token.cumul_size(); i++){
        value = token.cumul(i);
        if(value==1){
            break;
        }
        got_share.push_back((value-1)%257);
    }
    string hex_share = convertToHex(got_share, node_id, t);


    //copyString(std::string source, char* destination);
    char key[100];
    char share[MAX_SIZE_VALUE];
    copyString(token.key(), key);
    copyString(hex_share, share);

    //printf("\nRecreated share: %s\n", share);

    ecall_put(key, share);

}*/

void ecall_distributed_PI(const char *serialized_token){
    Token token;
    token.ParseFromString(serialized_token);
    distributed_polynomial_interpolation(token);
}

/*void recover_lost_share(string& key, vector<int> t_share_owners){

    int len_cumul = MAX_SIZE_VALUE;
    //printf("Enter init\n");
    Token token = init_token(key,t_share_owners, len_cumul);
    
    string serialized_token__;
    token.SerializeToString(&serialized_token__);
    //ocall_print_token(serialized_token__.c_str());
    
    //printf("Enter distributed pol\n");
    distributed_polynomial_interpolation(token);
    //printf("Success of distributed polynomial interpolation\n");
    int token_owner_id = t_share_owners.back();
    //printf("token_owner_id: %d\n",token_owner_id);

    //get token from its last owner
    char serialized_token[MAX_SIZE_SERIALIZED_TOKEN];
    memset(serialized_token, 'A', MAX_SIZE_SERIALIZED_TOKEN-1);
    ocall_get_tokens(&token_owner_id, serialized_token);

    //store share
    store_share(serialized_token);
}*/

void delete_last_share(int potential_last_share_owner, string key){
    ocall_delete_last_share(&potential_last_share_owner, key.c_str());
}



/*void ecall_recover_lost_shares(){
    vector<string> splitted_key_potential_last_share_owner_t_shares_owners;
    string key;
    string potential_last_share_owner;
    vector<int> t_shares_owners;

    int size_keys;
    ocall_get_size_keys(&size_keys);

    char lost_key[100];
    string lost_key_with_potential_last_share_owner_and_t_shares_owners;


    for(int i=0; i<size_keys; i++){
    //for(string lost_key_with_potential_last_share_owner_and_t_shares_owners: lost_keys_with_potential_last_share_owner_and_t_shares_owners){
        memset(lost_key, 'A', 99);
        ocall_get_lost_key(lost_key);
        lost_key_with_potential_last_share_owner_and_t_shares_owners = lost_key;

        splitted_key_potential_last_share_owner_t_shares_owners = splitString(lost_key_with_potential_last_share_owner_and_t_shares_owners, '|');
       
        key = splitted_key_potential_last_share_owner_t_shares_owners[0];
        potential_last_share_owner = splitted_key_potential_last_share_owner_t_shares_owners[1];
        //printf("jazet 3\n");
        t_shares_owners.clear();
        for (string& share_owner: splitString(splitted_key_potential_last_share_owner_t_shares_owners[2], ',')){
            t_shares_owners.push_back(stoi(share_owner));
        }
        //printf("jazet 4\n");
        recover_lost_share(key, t_shares_owners);

        if(potential_last_share_owner != "null"){
             delete_last_share(stoi(potential_last_share_owner), key);
        }
        //printf("khlass\n");

        //break; //just for example
        
        
    }
    //lost_keys_with_potential_last_share_owner_and_t_shares_owners.clear(); //commented because of not using add_lost_keys();
    ocall_flush_lost_keys_list();
  
}*/

void ecall_get_tokens(int* token_initiator_id, char* serialized_token){
   
    strncpy(serialized_token, temp_token.c_str(), strlen(serialized_token));
    temp_token = "";
    
}

void ecall_get_batch_keys_shares(int* node_id, 
                                 int* s_up_ids_array, unsigned cnt, 
                                 int* batch_size_key, 
                                 char* lost_keys_shares, 
                                 int* n_records){
   printf("hellooo");                                 
    
}

void ecall_get_number_of_keys(int* number_of_keys){

    *number_of_keys = myMap.size();
}

//ecall_generate_polynomial([in] int* s_new, [in, count=cnt] int* s_up_ids_array, [out, count=cnt] int* values, unsigned cnt);
void ecall_generate_polynomial(int* s_new, int* s_up_ids_array, int* values, unsigned cnt){ //Generate polynomial and returns its shares
    poly_R = static_cast<int*>(malloc(sizeof(int) * t));
    gener_R(t, *s_new, poly_R);
    printf("Generated polynomial coefs: %d %d %d\n", poly_R[0], poly_R[1], poly_R[2]);
    
    stored_polynomial_shares[node_id] = R(poly_R ,t, node_id);
    printf("Own polynomial share (%d , %d)\n", node_id, stored_polynomial_shares[node_id]);

    for(int i=0; i<cnt; i++){
        values[i] = R(poly_R, t, s_up_ids_array[i]);
        printf("share: (%d , %d)\n", s_up_ids_array[i], values[i]);
    }

}


void ecall_store_polynomial_share(int* source_node, int* value){
    stored_polynomial_shares[*source_node] = *value;
    printf("From node %d received polynomial share %d\n", *source_node, *value);
}