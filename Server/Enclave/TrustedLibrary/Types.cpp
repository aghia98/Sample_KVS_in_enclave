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


/* Test Basic Types */

#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"
#include <string>
#include <vector>
#include <algorithm>
#include <map> // Secure Key-Value Store

using namespace std;
// Declare myMap as a global variable
map<string, string> myMap;

/* used to eliminate `unused variable' warning */
#define UNUSED(val) (void)(val)

#define ULP 2


/*char* convertCString(std::string str) {
    char* cstr = new char[str.length() + 1];  // +1 for null-terminator
    strcpy(cstr, str.c_str());
    return cstr;
}*/

/*void copyString(std::string source, char* destination) {
    for (std::size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}*/

 


void ecall_put(char key[], char val[]){
    string key_string(key);
    string val_string(val);
    myMap[key_string]=val;

}

void ecall_get(char key[], char* val){
  string key_string(key);
  string ex="server650";
  
  auto iterator = myMap.find(key_string);
  string source;
  if(iterator != myMap.end()){
    source = iterator->second;
    //char destination[410];
    //copyString(source, destination); 
    
    strncpy(val, source.c_str(), strlen(val));
    //val = destination;
    
  }else{
    //val="NOT_FOUND";
    strncpy(val, "NOT_FOUND", strlen(val));
  } 

}

void ecall_delete(char key[]){
  string key_string(key);
  myMap.erase(key_string);
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

void ecall_share_lost_keys(int* node_id, char* lost_keys) {
  strncpy(lost_keys, "TBDDDDDDDDDDDDDDDD", strlen(lost_keys));

}