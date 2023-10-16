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
#include <vector>
#include <algorithm>
#include <map> // Secure Key-Value Store
#include <set>

//#include "ss-combine.h"

#include "token.pb.h" 
//***********************************************
//***********************************************

using namespace std;
using namespace token;
// Declare myMap as a global variable
map<string, string> myMap;
set<string> lost_keys_with_potential_last_share_owner_and_t_shares_owners;
int t;
int n;
int node_id;
string temp_token;

/* used to eliminate `unused variable' warning */
#define UNUSED(val) (void)(val)

#define ULP 2


/*char* convertCString(std::string str) {
    char* cstr = new char[str.length() + 1];  // +1 for null-terminator
    strcpy(cstr, str.c_str());
    return cstr;
}*/

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
    hex[2] = '\0'; // Null-terminate the string
}

string convertToHex(const vector<int>& input, int x_share, int t) {
    string result;
    
    char hex[3];

    intToHex(x_share, hex);
    result += hex[0];
    result += hex[1];

    intToHex(t, hex);
    result += hex[0];
    result += hex[1];

    result += "AA";

    for (int i = 0; i < input.size(); i++) {
        intToHex(input[i], hex);
        result += hex[0];
        result += hex[1];
    }

    return result;
}

void copyString(std::string source, char* destination) {
    for (std::size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
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
    myMap[key_string]=val;
}

void ecall_get(char key[], char* val){
  string key_string(key);
  
  auto iterator = myMap.find(key_string);
  string source;
  if(iterator != myMap.end()){
    source = iterator->second;
    //char destination[410];
    //copyString(source, destination); 
    
    strncpy(val, source.c_str(), strlen(val));
  }else{
    strncpy(val, "NOT_FOUND", strlen(val));
  } 
  //delete key;

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

void ecall_add_lost_keys(char keys_with_last_share_owner[]){

    if(strcmp(keys_with_last_share_owner,"null\n")!=0){
        string token;
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
}
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
    string serialized_token;

    token.SerializeToString(&serialized_token);
    //ocall_print_token(serialized_token.c_str());

    if(node_id!=token.initiator_id()){
        //TBD: compute the partial sum
        //get value
        char share[410];
        memset(share, 'A', 409);
        char token_key_char[410];
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
    }
}

void ecall_distributed_PI(const char *serialized_token){
    Token token;
    token.ParseFromString(serialized_token);
    distributed_polynomial_interpolation(token);
}

void store_share(const char *serialized_token){
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

    char token_key_char[100];

    //copyString(std::string source, char* destination);
    char key[100];
    char share[410];
    copyString(token.key(), key);
    copyString(hex_share, share);

    printf("\nRecreated share: %s\n", share);

    ecall_put(key, share);

    //*************************************************
    char value_[410];
    memset(value_, 'A', 409);
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    char* cstring_k = "Secret_1";
    ecall_get(cstring_k, value_);
}

void recover_lost_share(string& key, vector<int> t_share_owners){
    int len_cumul = 410;
    Token token = init_token(key,t_share_owners, len_cumul);
    
    /*string serialized_token__;
    token.SerializeToString(&serialized_token__);
    ocall_print_token(serialized_token__.c_str());*/
    
    distributed_polynomial_interpolation(token);
    //printf("Success of distributed polynomial interpolation\n");
    int token_owner_id = t_share_owners.back();
    //printf("token_owner_id: %d\n",token_owner_id);

    //get token from its last owner
    char serialized_token[1000];
    memset(serialized_token, 'A', 999);
    ocall_get_tokens(&token_owner_id, serialized_token);

    //store share
    store_share(serialized_token);
}

void delete_last_share(int potential_last_share_owner, string key){
    ocall_delete_last_share(&potential_last_share_owner, key.c_str());
}

void ecall_recover_lost_shares(){
    vector<string> splitted_key_potential_last_share_owner_t_shares_owners;
    string key;
    string potential_last_share_owner;
    vector<int> t_shares_owners;
    
    for(string lost_key_with_potential_last_share_owner_and_t_shares_owners: lost_keys_with_potential_last_share_owner_and_t_shares_owners){
        //printf("%s\n",lost_key_with_potential_last_share_owner_and_t_shares_owners);
        splitted_key_potential_last_share_owner_t_shares_owners = splitString(lost_key_with_potential_last_share_owner_and_t_shares_owners, '|');
        
        key = splitted_key_potential_last_share_owner_t_shares_owners[0];
        
        potential_last_share_owner = splitted_key_potential_last_share_owner_t_shares_owners[1];
        
        t_shares_owners.clear();
        for (string& share_owner: splitString(splitted_key_potential_last_share_owner_t_shares_owners[2], ',')){
            t_shares_owners.push_back(stoi(share_owner));
        }

        recover_lost_share(key, t_shares_owners);

        if(potential_last_share_owner != "null"){
             delete_last_share(stoi(potential_last_share_owner), key);
        }

        //break; //just for example
        
        /*printf("Key: %s\n",key.c_str());
        printf("potential last share owner: %s\n", potential_last_share_owner.c_str());
        printf("t_shares_owners: %s\n", t_shares_owners.c_str()); */
    }
    //flush lost_keys_with_potential_last_share_owner_and_t_shares_owners
    lost_keys_with_potential_last_share_owner_and_t_shares_owners.clear();
    
}

void ecall_get_tokens(int* token_initiator_id, char* serialized_token){
   
    strncpy(serialized_token, temp_token.c_str(), strlen(serialized_token));
    temp_token = "";
    
}

