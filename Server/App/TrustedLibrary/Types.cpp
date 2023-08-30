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


#include "../App.h"
#include "Enclave_u.h"
#include <string>
#include <cstring>
#include <iostream>
#include <vector>

#include <sstream>

using namespace std;

/* edger8r_type_attributes:
 *   Invokes ECALLs declared with basic types.

 
 */

char* convertCString(std::string str) {
    char* cstr = new char[str.length() + 1];  // +1 for null-terminator
    strcpy(cstr, str.c_str());
    return cstr;
}

void put(string k, string v)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_put(global_eid, convertCString(k), convertCString(v));
    if (ret != SGX_SUCCESS)
        abort();

}

string get(string k){
    //char* value;
    char value[410];
    memset(value, 'A', 409);
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    char* cstring_k = convertCString(k);
    ret = ecall_get(global_eid, cstring_k, value);
    if (ret != SGX_SUCCESS)
        abort();
    delete cstring_k;

    string value_string(value); 

    return value_string;
}

void delete_(string key){
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_delete(global_eid, convertCString(key));
    if (ret != SGX_SUCCESS)
        abort();
}

set<string> share_lost_keys(int node_id, vector<int> s_up_ids){
    char lost_keys_with_potential_last_share_owner[10000]; //*******************************************assuming there is max 100 lost keys per node*********************
    memset(lost_keys_with_potential_last_share_owner, 'A', 999);

    int* s_up_ids_array = &s_up_ids[0];

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    //**************************************6
    ret = ecall_share_lost_keys(global_eid, &node_id, s_up_ids_array, s_up_ids.size(), lost_keys_with_potential_last_share_owner); // add S_up as input parameter
    if (ret != SGX_SUCCESS)
        abort();

    cout << lost_keys_with_potential_last_share_owner << endl;
    //*************************post-process************************
    
    set<string> keys;

    if(strlen(lost_keys_with_potential_last_share_owner) != 0){
        istringstream iss(lost_keys_with_potential_last_share_owner);
        string token;
        

        while (std::getline(iss, token, '\n')) {
            if (!token.empty()) {
                keys.insert(token);
            }
        } 

        /*for (const std::string& str : keys) {
            std::cout << str << std::endl;
        }*/
    }
    
    return keys;
} 

void add_lost_keys_in_enclave(const set<string>& local_lost_keys_set){
    string lost_keys_string="";
    for (const auto& key_with_last_share_owner : local_lost_keys_set) {
        lost_keys_string += key_with_last_share_owner+"\n";
    } 

    //cout << lost_keys_string << endl;

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_add_lost_keys(global_eid, convertCString(lost_keys_string)); 
    if (ret != SGX_SUCCESS)
        abort();
}

void recover_lost_shares(){
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_recover_lost_shares(global_eid);
    if (ret != SGX_SUCCESS)
        abort(); 
}
