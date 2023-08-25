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


#include <map>

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

void copyString(std::string source, char* destination) {
    for (std::size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}



/* ecall_type_int:
 *   [int] value passed by App.
 */
void ecall_put(char key[], char val[]){
    string key_string(key);
    string val_string(val);
    myMap[key_string]=val;

}

void ecall_get(char key[], char** val){
  string key_string(key);
  
  auto iterator = myMap.find(key_string);
  if(iterator != myMap.end()){
    string source = iterator->second;
    char destination[200];

    copyString(source, destination); 
    *val = destination;
  }else{
    *val="buggg";
  }

}

void ecall_rebuild_secret(char combined_shares[], char* val){
  //*val="yeaaaaaaaaah";
  char* secret = extract_secret_from_share_strings(combined_shares);
  strncpy(val, secret, strlen(val));
  free(secret);
  //*val = extract_secret_from_share_strings(combined_shares);

}
