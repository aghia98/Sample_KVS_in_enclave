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


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>  
#include <arpa/inet.h> //check if the grpc server port is open

# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"

#include "../../gRPC_module/grpc_client.h"
#include "../../SS_no_gmp_module/ss-worker.h"
#include "../../HRW_hashing_module/hrw.h"

#include "Enclave_u.h"

using namespace std;

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
    {
        SGX_ERROR_MEMORY_MAP_FAILURE,
        "Failed to reserve memory for the enclave.",
        NULL
    },
};


class KVSClient {
 public:
  KVSClient(std::shared_ptr<Channel> channel): stub_(KVS::NewStub(channel)) {}

  string Get(const string k) {
    Key key;
    key.set_key(k);

    Value reply;

    ClientContext context;

    Status status = stub_->Get(&context, key, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.value();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  unique_ptr<KVS::Stub> stub_;
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}

void copyString(string source, char* destination) {
    for (size_t i = 0; i < source.length(); ++i) {
        destination[i] = source[i];
    }
    destination[source.length()] = '\0'; // Append null character to terminate the string
}

bool isPortOpen(const std::string& ipAddress, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        // Failed to create socket
        return false;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress.c_str(), &(serverAddress.sin_addr)) <= 0) {
        // Invalid address format
        close(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        // Connection failed, port is closed
        close(sock);
        return false;
    }

    // Connection successful, port is open
    close(sock);
    return true;
}

vector<int> get_ids_of_N_active(vector<int>& list_of_N_ports, int offset){
  vector<int> result;
  for(int port: list_of_N_ports){
    if (isPortOpen("127.0.0.1", port)) result.push_back(port-offset);
  }

  return result;
}

string get_shares(vector<int> ids_of_N_active, string secret_id, string ip_address, int t){
    int port;
    int offset=50000;
    //int got_shares=0;
    KVSClient* kvs;
    string shares = "";
    string reply;
    string fixed = ip_address+":";
    vector<string> strings_with_id_of_N_active = convert_ids_to_strings_with_id(ids_of_N_active, "server");
    vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,secret_id); //Order according to HRW
    pair<string, uint32_t> pair; 

    for(int i=0; i<t; i++){
        pair = ordered_strings_with_id_to_hash[i];
        port = extractNumber(pair.first) + offset;
        kvs = new KVSClient(grpc::CreateChannel(fixed+to_string(port), grpc::InsecureChannelCredentials()));
        shares += kvs->Get(secret_id)+'\n'; 
        delete kvs;
    }

  return shares;
}




ABSL_FLAG(uint16_t, port, 50001, "Server port for the service");

/* Application entry */
int SGX_CDECL main(int argc, char *argv[]){ //--address <address> --secret_id <secret_id> -t <n>
 
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }

    vector<int> list_of_N_ports = {50001,50002,50003,50004,50005}; int offset=50000;
    vector<int> ids_of_N_active;
    vector<string> strings_with_id_of_N_active;
    vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;
    int n;
    int t;
    //int port;
    string ip_address;
    string secret_id;

    struct option long_options[] = {
        {"address", required_argument, nullptr, 'a'},
        {"secret_id", required_argument, nullptr, 's'},
        {nullptr, 0, nullptr, 0}
    };

    int option;
    while ((option = getopt_long(argc, argv, "t:", long_options, nullptr)) != -1) {
        switch (option) {
            case 'a':
                ip_address = optarg;
                break;
            case 's':
                secret_id = optarg;
                break;
            case 't':
                t = atoi(optarg);
                break;
            default:
                cerr << "Usage: " << argv[0] << " --address <address> --secret_id <secret_id> -t <t> -n <n>" << endl; //TBD: --port_init should be dynamically found
                return 1;
        }
    }
    ids_of_N_active = get_ids_of_N_active(list_of_N_ports, offset);
    if(ids_of_N_active.size() >= t){
        string shares_string = get_shares(ids_of_N_active, secret_id, ip_address, t);
        char* combined_shares = static_cast<char*>(malloc((shares_string.length()+1)*sizeof(char)));
        cout << shares_string << endl;

        copyString(shares_string, combined_shares); 
        string secret = rebuild_secret(combined_shares);
        cout << secret << endl;
        free(combined_shares);

    }else{
        cout << "less than t=" << t << " SMS nodes are available. Please retry later." << endl;
    } 
  
    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);

    printf("Worker closed \n");
    
    return 0;
}

