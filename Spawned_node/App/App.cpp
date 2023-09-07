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

# include <unistd.h>
# include <pwd.h>
#include <arpa/inet.h> //check if the grpc server port is open
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"

#include "../../gRPC_module/grpc_server.h"
#include "../../gRPC_module/grpc_client.h"

#include "Enclave_u.h"

using namespace token;

//Global variables;
int node_id_global;
int t_global;
int n_global;

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


// Logic and data behind the server's behavior.
class KVSServiceImpl final : public KVS::Service {

    Status Get(ServerContext* context, const Key* key, Value* value) override {
        value->set_value(get(key->key()));
        
        return Status::OK;
    }


    Status Put(ServerContext* context, const KV_pair* request, Value* response) override {
        put(request->key(), request->value());
        response->set_value("PUT_SUCCESS");
        return Status::OK;
  }

    Status Delete(ServerContext* context, const Key* key, Value* response) override{
        delete_(key->key());
        response->set_value("DELETE_SUCCESS");
        return Status::OK;
  }

    Status Share_lost_keys(ServerContext* context, const New_id_with_S_up_ids* request, Lost_keys* response) override {
        
        vector<int> s_up_ids;

        for (const auto& s_up_id : request->s_up_ids()) {
            s_up_ids.push_back(s_up_id);
        }

        //**************************5
        set<string> lost_keys = share_lost_keys(request->new_id(), s_up_ids);
        
        //checking result
        for (set<string>::iterator it = lost_keys.begin(); it != lost_keys.end(); ++it) {
            Key* key = response->add_keys();
            key->set_key(*it);
        }

        return Status::OK;
    }
};


//set<string> global_lost_keys_set; //global variable
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

    string Put(const string k, const string v) {
        // Follows the same pattern as SayHello.
        KV_pair request;
        request.set_key(k);
        request.set_value(v);
        Value reply;
        ClientContext context;

        // Here we can use the stub's newly available method we just added.
        Status status = stub_->Put(&context, request, &reply);
        if (status.ok()) {
            return reply.value();
        } else {
            cout << status.error_code() << ": " << status.error_message()
                    << endl;
            return "RPC failed";
        }
    }
     
    int Share_lost_keys(int id, vector<int> s_up_ids){
        New_id_with_S_up_ids request;
        request.set_new_id(id); // Replace with the desired ID value
        for(auto& s_up_id : s_up_ids) request.add_s_up_ids(s_up_id);
        

        // Create a Lost_keys response
        Lost_keys response_local;
        set<string> local_lost_keys_set;

        ClientContext context;
        Status status = stub_->Share_lost_keys(&context, request, &response_local); //***************3
        string lost_key;

        if (status.ok()) {
            for (const auto& key : response_local.keys()) {
                lost_key=key.key();
                local_lost_keys_set.insert(lost_key);
                //global_lost_keys_set.insert(lost_key);
            }
            //send lost keys to enclave
            add_lost_keys_in_enclave(local_lost_keys_set);

        } else {
            std::cerr << "RPC failed";
        }

        /*for (const auto& key : global_lost_keys_set) {
                cout << key << endl;
        }*/
        return 0;
  }

    string Partial_Polynomial_interpolation(Token token) {

        Value reply;
        ClientContext context;

        CompletionQueue cq;
        Status status;

        std::unique_ptr<ClientAsyncResponseReader<Value> > rpc(
        stub_->AsyncPartial_Polynomial_interpolation(&context, token, &cq));
        rpc->Finish(&reply, &status, (void*)1);

        void* got_tag;
        bool ok = false;
        cq.Next(&got_tag, &ok);
        if (ok && got_tag == (void*)1) {
            if (status.ok()) {
                return reply.value();
            } else {
                std::cout << status.error_code() << ": " << status.error_message() << std::endl;
                return "RPC failed";
            }
        }

        /*status = stub_->Partial_Polynomial_interpolation(&context, token, &reply);
        if (status.ok()) return reply.value();
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC failed"; */
  }

  string Get_tokens(){
        Node_id request;
        List_tokens list_tokens;

        

        int source_id = node_id_global;

        request.set_id(source_id);

        ClientContext context;
        Status status = stub_->Get_tokens(&context, request, &list_tokens);

        if (status.ok()){
            Token token = list_tokens.tokens(0);
            string serialized_token;
            token.SerializeToString(&serialized_token);
            //printf("halllo\n");

            
            //printf("%s\n",serialized_token.c_str());
            

            //ret = ecall_store_share(global_eid, serialized_token.c_str());

            /*if (ret != SGX_SUCCESS)
                abort(); */
            //printf("halllo2\n");  

            return serialized_token;
        }else{
            return "null";
        }
    }

 private:
  unique_ptr<KVS::Stub> stub_;
};





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
    printf("%s", str);
    fflush(stdout);
}

void ocall_print_token(const char *serialized_token){
    Token token;
    

    token.ParseFromString(serialized_token);
    printf("***************Received Token begin*********************\n");
    printf("initiator_id: %d\n", token.initiator_id());
    printf("key: %s\n", token.key().c_str());
    printf("passes: %d\n", token.passes());

    printf("cumul:");
    for (int i = 0; i < 20; ++i) {
        printf(" %d", token.cumul(i));
    }
    printf("\n");

    printf("path:");
    for (int i = 0; i < token.path_size(); ++i) {
        printf(" %d", token.path(i));
    }
    printf("\n");
    printf("***************Received Token end*********************\n");

}

void ocall_send_token(const char *serialized_token, int* next_node_id){
    //printf("next node id: %d\n", *next_node_id);

    KVSClient* kvs;
    Token token; 
    int offset = 50000;

    token.ParseFromString(serialized_token);
    kvs = new KVSClient(grpc::CreateChannel("localhost:"+to_string(offset+ (*next_node_id)) , grpc::InsecureChannelCredentials()));
    kvs->Partial_Polynomial_interpolation(token);
    delete kvs;
}

void ocall_get_tokens(int* node_id, char* serialized_token){
    KVSClient* kvs;
    int offset = 50000;
    
    kvs = new KVSClient(grpc::CreateChannel("localhost:"+to_string(offset+ (*node_id)) , grpc::InsecureChannelCredentials()));
    string serialized_token_ = kvs->Get_tokens();
    delete kvs;

    //printf("and thennn ?\n");

    strncpy(serialized_token, serialized_token_.c_str(), strlen(serialized_token));

}


//***********************************************************************************************

void test_share_lost_keys(int current_port, int offset, int n_servers, int starting_port){
    KVSClient* kvs;
    int current_id = current_port - offset;

    vector<int> S_up_ids;
    for(int i=0; i<n_servers; i++){

        int port = starting_port+i;

        if(port!=current_port){
            if(isPortOpen("127.0.0.1", port)) {
                
                S_up_ids.push_back(port-offset);
            }
        }
        
    }
    for(int s_up_id : S_up_ids){
        kvs = new KVSClient(grpc::CreateChannel("localhost:"+to_string(offset+s_up_id) , grpc::InsecureChannelCredentials()));
        kvs->Share_lost_keys(current_id, S_up_ids); //******************************2
        delete kvs;
    }
    

    /*for (const auto& key : global_lost_keys_set) {
        cout << key << endl;
    }*/
}

void recover_lost_shares_wrapper(){
    recover_lost_shares();
}

void RunServer(uint16_t port) {
 
    std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
    KVSServiceImpl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "SMS node listening on " << server_address << std::endl;
    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

ABSL_FLAG(uint16_t, port, 50001, "Server port for the service");
ABSL_FLAG(uint16_t, t, 3, "number of necessary shares");
ABSL_FLAG(uint16_t, n, 5, "number of all shares");

/* Application entry */
int SGX_CDECL main(int argc, char *argv[]) // ./app --port 50001
{   
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }

    absl::ParseCommandLine(argc, argv);

    uint16_t port = absl::GetFlag(FLAGS_port);
    int offset = 50000;
    int t = absl::GetFlag(FLAGS_t);
    int n = absl::GetFlag(FLAGS_n);
    int node_id = port - offset;

    t_global = t;
    n_global = n;
    node_id_global = node_id;

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_send_params_to_enclave(global_eid, &node_id, &t, &n);
    if (ret != SGX_SUCCESS)
        abort();
    test_share_lost_keys(port, offset, 5, 50001);//****************1
    recover_lost_shares_wrapper();

    RunServer(port);
    sgx_destroy_enclave(global_eid);
    printf("Server closed \n");
    

    return 0;
}

