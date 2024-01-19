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
#include <chrono>
#include <mutex>

# include <unistd.h>
# include <pwd.h>
#include <arpa/inet.h> //check if the grpc server port is open
#define MAX_PATH FILENAME_MAX
#define BATCH_SIZE_KEY 100

#define MAX_SIZE_VALUE 1032
#define MAX_SIZE_SERIALIZED_TOKEN 1532
#define NUM_THREADS 16


#include "sgx_urts.h"
#include "App.h"

#include "../../gRPC_module/grpc_server.h"
#include "../../gRPC_module/grpc_client.h"

#include "../../Token_module/grpc_server.h"
#include "../../Token_module/grpc_client.h"

#include "../../HRW_hashing_module/hrw.h"

#include <fstream>
#include "../../json/json.hpp"
using json = nlohmann::json;


#include "Enclave_u.h"
#include <thread>
#include <chrono>
#include <csignal>



//Global variables;
int node_id_global;
int t_global;
int n_global;
map<int, string> id_to_address_map;
map<string, string> myMap;

set<string> keys_set;
set<string> lost_keys_with_potential_last_share_owner_and_t_shares_owners; set<string>::iterator it_lost_keys;

int cpt = 0;

int default_sms_port = 50000;
std::mutex putMutex_;


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



map<int, string> parse_json(const string& file_location){

  ifstream file(file_location);
  if (!file.is_open()) {
      std::cerr << "Failed to open JSON file." << std::endl;
  }

  json jsonData;
  try {
      file >> jsonData;
  } catch (json::parse_error& e) {
      std::cerr << "JSON parsing error: " << e.what() << std::endl;
  }

  map<int, std::string> resultMap;

  for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
      int key = std::stoi(it.key());
      std::string value = it.value();
      resultMap[key] = value;
  }

  return resultMap;

}

class KVSServiceImpl {
    public:
        KVSServiceImpl(){}
        
        template <typename T>
        static void createNew(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq) {
            new T(parent, service, cq);
        }

        template <typename T>
        static void createNew2(KVSServiceImpl& parent, tokengrpc::Token::AsyncService& service, ServerCompletionQueue& cq) {
            new T(parent, service, cq);
        }

        class RequestBase{
            public:
                RequestBase(KVSServiceImpl& parent, ServerCompletionQueue& cq)
                    :parent_{parent}, cq_{cq} {}

                virtual ~RequestBase() = default;

                // The state-machine
                virtual void proceed(bool ok) = 0;


            protected:
                static size_t getNewReqestId() noexcept {
                    static size_t id = 0;
                    return ++id;
                }


                KVSServiceImpl& parent_;
                ServerCompletionQueue& cq_;
                ServerContext ctx_;
                const size_t rpc_id_ = getNewReqestId();
        };


        class RequestBaseKVS : public RequestBase{
            public:
                RequestBaseKVS(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq)
                    : RequestBase(parent, cq), service_kvs{service} {}

                virtual ~RequestBaseKVS() = default;

                // The state-machine
                virtual void proceed(bool ok) = 0;


            protected:
                //KVSServiceImpl& parent_;
                keyvaluestore::KVS::AsyncService& service_kvs;
                //ServerCompletionQueue& cq_;
                //ServerContext ctx_;
                //const size_t rpc_id_ = getNewReqestId();
        };

        class RequestBaseToken : public RequestBase{
            public:
                RequestBaseToken(KVSServiceImpl& parent, tokengrpc::Token::AsyncService& service, ServerCompletionQueue& cq)
                    : RequestBase(parent, cq), service_token{service} {}

                virtual ~RequestBaseToken() = default;

                // The state-machine
                virtual void proceed(bool ok) = 0;


            protected:
                tokengrpc::Token::AsyncService& service_token;
        };


        class PutRequest : public RequestBaseKVS {
            public:

                PutRequest(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBaseKVS(parent, service, cq) {

                    service_kvs.RequestPut(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {
                    if (state_ == CREATE) {

                        //cpt++;
                        //if(cpt==10100) sgx_destroy_enclave(global_eid);
                        
                        createNew<PutRequest>(parent_, service_kvs, cq_);

                        /*myMap[request_.key()] = request_.value();
                        state_ = FINISH;
                        responder_.Finish(reply_, Status::OK, this);*/
                        
                        
                        keys_set.insert(request_.key());
                        //std::thread([this]() {
                            //auto start_time = std::chrono::high_resolution_clock::now();
                            myMap[request_.key()] = request_.value();
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);
                            //auto end_time = std::chrono::high_resolution_clock::now();
                            //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            //std::cout << "Latency Put: " << duration.count() << " ms" << std::endl;*/

                            
                            //auto start_time = std::chrono::high_resolution_clock::now();
                            /*{
                                std::lock_guard<std::mutex> lock(putMutex_);
                                put(request_.key(), request_.value());
                            }
                            //reply_.set_value("PUT_SUCCESS");
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);*/
                            //auto end_time = std::chrono::high_resolution_clock::now();
                            //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            //std::cout << "Latency Put: " << duration.count() << " ms" << std::endl; 
                        //}).detach();
 
                    } else { //state == FINISH
                        delete this; 
                    } 
                    
                }

            private:
                keyvaluestore::KV_pair request_;
                keyvaluestore::Value reply_;
                grpc::ServerAsyncResponseWriter<keyvaluestore::Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };


        class GetRequest : public RequestBaseKVS{
            public:

                GetRequest(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBaseKVS(parent, service, cq) {


                    service_kvs.RequestGet(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew<GetRequest>(parent_, service_kvs, cq_);

                        /*reply_.set_value(myMap[request_.key()]);
                        state_ = FINISH;
                        responder_.Finish(reply_, Status::OK, this);*/

                        //std::thread([this]() {
                            
                            //auto start_time = std::chrono::high_resolution_clock::now();
                            reply_.set_value(myMap[request_.key()]);
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);
                            //auto end_time = std::chrono::high_resolution_clock::now();
                            //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            //std::cout << "Latency Get: " << duration.count() << " ms" << std::endl;*/


                            //auto start_time = std::chrono::high_resolution_clock::now();
                            //std::lock_guard<std::mutex> lock(getMutex);
                            /*reply_.set_value(get(request_.key()));
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);*/
                            //auto end_time = std::chrono::high_resolution_clock::now();
                            //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            //std::cout << "Latency Get: " << duration.count() << " ms" << std::endl;
                        //}).detach();

                        
                    } else  {
                        delete this; 
                    } 
                }

            private:
                keyvaluestore::Key request_;
                keyvaluestore::Value reply_;
                grpc::ServerAsyncResponseWriter<keyvaluestore::Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };

        class DeleteRequest : public RequestBaseKVS{
            public:

                DeleteRequest(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBaseKVS(parent, service, cq) {

                    service_kvs.RequestDelete(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew<DeleteRequest>(parent_, service_kvs, cq_);


                        std::thread([this]() {
                            keys_set.erase(request_.key());
                            delete_(request_.key());
                            reply_.set_value("DELETE_SUCCESS");
                            
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);
                        }).detach();

                        
                    } else {
                        delete this;  
                    } 
                }

            private:
                keyvaluestore::Key request_;
                keyvaluestore::Value reply_;
                grpc::ServerAsyncResponseWriter<keyvaluestore::Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };

        /*class Share_lost_keysRequest : public RequestBaseKVS{
            public:

                Share_lost_keysRequest(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBaseKVS(parent, service, cq) {


                    service_kvs.RequestShare_lost_keys(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew<Share_lost_keysRequest>(parent_, service_kvs, cq_);

                        vector<int> s_up_ids;

                        for (const auto& s_up_id : request_.s_up_ids()) {
                            s_up_ids.push_back(s_up_id);
                        }
                        //int* s_up_ids_array = &s_up_ids[0];
                        
                        //**************************5
                        set<string> lost_keys = share_lost_keys(request_.new_id(), s_up_ids);
                        
                        if(lost_keys.empty()){
                            keyvaluestore::Key* key = reply_.add_keys();
                            key->set_key("null");
                        }else{
                            for (set<string>::iterator it = lost_keys.begin(); it != lost_keys.end(); ++it) {
                                keyvaluestore::Key* key = reply_.add_keys();
                                key->set_key(*it);
                            }
                        }


                        state_ = FINISH;
                        responder_.Finish(reply_, Status::OK, this);

                    } else {
                        delete this;  
                    } 
                }

            private:
                keyvaluestore::New_id_with_S_up_ids request_;
                keyvaluestore::Lost_keys reply_;
                grpc::ServerAsyncResponseWriter<keyvaluestore::Lost_keys> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };*/

        class Share_lost_keysRequest : public RequestBaseKVS {
            public:

                Share_lost_keysRequest(KVSServiceImpl& parent, keyvaluestore::KVS::AsyncService& service, ServerCompletionQueue& cq)
                    : RequestBaseKVS(parent, service, cq) {

                    // Register this instance with the event-queue and the service.
                    // The first event received over the queue is that we have a request.
                    service_kvs.RequestShare_lost_keys(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {
                    if (state_ == CREATE){
                        sum = 0;
                        cout << "number of keys: " << keys_set.size() << endl;
                        cout << "begin send" << endl;
                        createNew<Share_lost_keysRequest>(parent_, service_kvs, cq_);
                        it = keys_set.begin();
                        if(it == keys_set.end()){ //empty set
                            state_ = FINISH;
                            responder_.Finish(Status::OK, this);
                        }else{ //1st write
                            for (const auto& s_up_id : request_.s_up_ids()) {
                                s_up_ids.push_back(s_up_id);
                            } 
                            keyvaluestore::Key* key;
                            set<string> lost_keys = share_lost_keys(request_.new_id(), BATCH_SIZE_KEY);
                            for(set<string>::iterator it_local = lost_keys.begin(); it_local != lost_keys.end(); it_local++){
                                key = reply_.add_keys();
                                key->set_key(*it_local);
                            }
                            
                            state_ = REPLYING;
                            sum += lost_keys.size();
                            //cout << "Size of lost_keys stream: " << lost_keys.size() << endl;
                            cout << "Sent: " << sum << "/" << keys_set.size() << endl;
                            responder_.Write(reply_, this);
                            reply_.Clear();
                        }
                    }else if (state_ == REPLYING){
                        if(it == keys_set.end()){
                            state_ = FINISH;
                            responder_.Finish(Status::OK, this);
                        }else{
                            keyvaluestore::Key* key;
                            set<string> lost_keys = share_lost_keys(request_.new_id(), BATCH_SIZE_KEY);
                            for(set<string>::iterator it_local = lost_keys.begin(); it_local != lost_keys.end(); ++it_local){
                                key = reply_.add_keys();
                                key->set_key(*it_local);
                            }
                            sum += lost_keys.size();
                            //cout << "Size of lost_keys stream: " << lost_keys.size() << endl;
                            cout << "Sent: " << sum << "/" << keys_set.size() << endl;
                            responder_.Write(reply_, this);
                            reply_.Clear();
                        }

                    }else { // state_ == FINISH 
                        cout << "end send" << endl;
                        delete this;
                    }
                }
        
            private:
                int sum;
                set<string>::iterator it;
                enum CallStatus { CREATE, FINISH, REPLYING };
                CallStatus state_ = CREATE;
                ServerAsyncWriter<keyvaluestore::Lost_keys> responder_{&ctx_};
                keyvaluestore::New_id_with_S_up_ids request_;
                keyvaluestore::Lost_keys reply_;
                vector<int> s_up_ids;

                set<string> share_lost_keys(int node_id, int n_records){
                    vector<string> strings_with_id_of_N_active;
                    vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;
                    string word = "server";
                    string keys="";
                    strings_with_id_of_N_active = convert_ids_to_strings_with_id(s_up_ids, word);
                    set<string> to_return;

                    int cpt = 0;
                    int cnt = s_up_ids.size();
                    set<string>::iterator it_local;

                    for (it_local = it; it_local != keys_set.end(); it_local++){
                        string  k = *it_local; //Secret_n
                        ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,k); //Order according to HRW
                        string spawned_node_id = word+to_string(node_id); //serverX
                        string spawned_node_to_hash = spawned_node_id+k; //ServerX+k
                        uint32_t spawned_node_hash_wrt_k = jenkinsHash(spawned_node_to_hash);
                        uint32_t n_th_node_hash;
                        string potential_last_share_owner_id;
                        if(cnt<n_global){
                            n_th_node_hash = 0;
                            potential_last_share_owner_id = "null"; // number of active nodes is smaller than n
                        }else{
                            n_th_node_hash = ordered_strings_with_id_to_hash[n_global-1].second;
                            potential_last_share_owner_id = to_string(extractNumber(ordered_strings_with_id_to_hash[n_global-1].first));
                        }

                        if(spawned_node_hash_wrt_k > n_th_node_hash){ //spawned_node is a neighbour for k
                            //gather at least t share owners
                            string t_shares_owners = "";

                            for(int i=0; i<t_global; i++){
                                t_shares_owners += to_string(extractNumber(ordered_strings_with_id_to_hash[i].first));
                                if(i<t_global-1){
                                    t_shares_owners +=",";
                                }
                            }
                            keys += k+"|"+potential_last_share_owner_id+"|"+t_shares_owners;
                        }else{
                            cout << "NOT SENT!!! " << spawned_node_hash_wrt_k << endl;
                        }
                        to_return.insert(keys);
                        keys = "";

                        cpt++;
                        if(cpt == n_records) break;
                    }

                    if(it_local != keys_set.end()){
                        it_local++;
                    }
                    it = it_local;
                    
                    return to_return;
                }
        };

        class Partial_Polynomial_interpolationRequest : public RequestBaseToken {
            public:

                Partial_Polynomial_interpolationRequest(KVSServiceImpl& parent, tokengrpc::Token::AsyncService& service, ServerCompletionQueue& cq): RequestBaseToken(parent, service, cq) {

                    service_token.RequestPartial_Polynomial_interpolation(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew2<Partial_Polynomial_interpolationRequest>(parent_, service_token, cq_);
                        
                        string serialized_token;
                        request_.SerializeToString(&serialized_token);
                    
                        sgx_status_t ret = SGX_ERROR_UNEXPECTED;
                        ret = ecall_distributed_PI(global_eid, serialized_token.c_str());
                        if (ret != SGX_SUCCESS)
                            abort();
                        
                        reply_.set_value("PARTIAL_INTERPOLATION_SUCCESS");
                        state_ = FINISH;
                        responder_.Finish(reply_, Status::OK, this);

                    } else {
                        delete this;  
                    } 
                }

            private:
                token::Token request_;
                tokengrpc::Value reply_;
                grpc::ServerAsyncResponseWriter<tokengrpc::Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };

        class Get_tokensRequest : public RequestBaseToken {
            public:

                Get_tokensRequest(KVSServiceImpl& parent, tokengrpc::Token::AsyncService& service, ServerCompletionQueue& cq): RequestBaseToken(parent, service, cq) {

                    service_token.RequestGet_tokens(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew2<Get_tokensRequest>(parent_, service_token, cq_);

                        char serialized_token[MAX_SIZE_SERIALIZED_TOKEN];
                        memset(serialized_token, 'A', MAX_SIZE_SERIALIZED_TOKEN-1);

                        sgx_status_t ret = SGX_ERROR_UNEXPECTED;
                        int source_ = request_.id();
                        ret = ecall_get_tokens(global_eid, &source_, serialized_token);
                        if (ret != SGX_SUCCESS)
                            abort();
                        token::Token token;
                        token.ParseFromString(serialized_token);

                        *(reply_.add_tokens()) = token;

                        state_ = FINISH;
                        responder_.Finish(reply_, Status::OK, this);

                        
                    } else {
                        delete this;  
                    } 
                }

            private:
                tokengrpc::Node_id request_;
                tokengrpc::List_tokens reply_;
                grpc::ServerAsyncResponseWriter<tokengrpc::List_tokens> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };


        void Run(uint16_t port) {
            std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

            grpc::ServerBuilder builder;
            builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
            builder.RegisterService(&service_kvs);
            builder.RegisterService(&service_token);

            //cq_ = builder.AddCompletionQueue();
            for (int i = 0; i < NUM_THREADS; ++i){
                cq_[i] = builder.AddCompletionQueue();
            }
            server_ = builder.BuildAndStart();
            std::cout << "Server listening on " << server_address << std::endl;

            //HandleRpcs();
            HandleRpcsMultiThreadsMultiQueues();
        }

        /*void HandleRpcs() {
            createNew<PutRequest>(*this, service_kvs, *cq_);
            createNew<GetRequest>(*this, service_kvs, *cq_);
            createNew<DeleteRequest>(*this, service_kvs, *cq_);
            createNew<Share_lost_keysRequest>(*this, service_kvs, *cq_);
            createNew2<Partial_Polynomial_interpolationRequest>(*this, service_token, *cq_);
            createNew2<Get_tokensRequest>(*this, service_token, *cq_);

            // The inner event-loop
            while(true) {
                bool ok = true;
                void *tag = {};

                const auto status = cq_->Next(&tag, &ok);

                switch(status) {
                    case grpc::CompletionQueue::NextStatus::GOT_EVENT:
                        {
                            auto request = static_cast<RequestBase *>(tag);
                            request->proceed(ok);
                        }
                        break;

                    case grpc::CompletionQueue::NextStatus::SHUTDOWN:
                        return;
                } // switch
            } // loop
        }*/

        void HandleRpcsMultiThreadsMultiQueues() {
            std::vector<std::thread> threads;
            for (int i = 0; i < NUM_THREADS ; ++i){
                std::cout << "Thread " << i+1 << " created " << std::endl;
                threads.emplace_back([this,i]() {

                    createNew<PutRequest>(*this, service_kvs, *cq_[i]);
                    createNew<GetRequest>(*this, service_kvs, *cq_[i]);
                    createNew<DeleteRequest>(*this, service_kvs, *cq_[i]);
                    createNew<Share_lost_keysRequest>(*this, service_kvs, *cq_[i]);
                    createNew2<Partial_Polynomial_interpolationRequest>(*this, service_token, *cq_[i]);
                    createNew2<Get_tokensRequest>(*this, service_token, *cq_[i]);

                    while (true) {
                        bool ok = true;
                        void *tag = {};
                        bool status;
                        status = cq_[i]->Next(&tag, &ok);
                        
                        switch (status) {
                            case grpc::CompletionQueue::NextStatus::GOT_EVENT: {
                                auto request = static_cast<RequestBase *>(tag);
                                request->proceed(ok);
                            } break;

                            case grpc::CompletionQueue::NextStatus::SHUTDOWN:
                                return;
                        }  // switch
                    }  // loop
                });

            }

            for (auto &thread : threads) {
                thread.join();
            }
        }

    private:
        keyvaluestore::KVS::AsyncService service_kvs;
        tokengrpc::Token::AsyncService service_token;
        //std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_[NUM_THREADS];
        std::unique_ptr<grpc::Server> server_;
};



class KVSClient {
 public:
    KVSClient(std::shared_ptr<Channel> channel): stub_(keyvaluestore::KVS::NewStub(channel)) {}

    string Get(const string k) {
        keyvaluestore::Key key;
        key.set_key(k);

        keyvaluestore::Value reply;

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
        keyvaluestore::KV_pair request;
        request.set_key(k);
        request.set_value(v);
        keyvaluestore::Value reply;
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

     string Delete(const string key){
        ClientContext context;
        keyvaluestore::Key request;
        request.set_key(key);
        keyvaluestore::Value reply;

        Status status = stub_->Delete(&context, request, &reply);

        if (status.ok()) {
            return reply.value();
        } else {
            cout << status.error_code() << ": " << status.error_message() << endl;
            return "RPC failed";
        }
    }
     
     int Share_lost_keys(int id, vector<int> s_up_ids){
        keyvaluestore::New_id_with_S_up_ids request;
        request.set_new_id(id); // Replace with the desired ID value
        for(auto& s_up_id : s_up_ids) request.add_s_up_ids(s_up_id);
        

        // Create a Lost_keys response
        keyvaluestore::Lost_keys response_local;
        //set<string> local_lost_keys_set;

        ClientContext context;
        std::unique_ptr<ClientReader<keyvaluestore::Lost_keys> > reader(stub_->Share_lost_keys(&context, request));
        string lost_key;
        int cpt= 0;
        cout << "begin read" << endl;
        while (reader->Read(&response_local)) {
            for (const auto& key : response_local.keys()) {
                lost_key = key.key();
                //cout << lost_key << endl;
                //local_lost_keys_set.insert(lost_key);
                lost_keys_with_potential_last_share_owner_and_t_shares_owners.insert(lost_key);
                cpt++;
            }
            //add_lost_keys_in_enclave(local_lost_keys_set);
            //printf("%d\n",cpt);
            //local_lost_keys_set.clear();
        }
        Status status = reader->Finish();
        it_lost_keys = lost_keys_with_potential_last_share_owner_and_t_shares_owners.begin();
        cout << "Recieved: " << cpt << " keys"<< endl;
        

        if (status.ok()) {
            cout << "Keys' Recovery succeded" << endl;
        }  else {
            std::cerr << "RPC failed";
        }


        return 0;
  }

 private:
  unique_ptr<keyvaluestore::KVS::Stub> stub_;
};

class TokenServiceImpl final : public tokengrpc::Token::Service {
        
    Status Partial_Polynomial_interpolation(ServerContext* context, const token::Token* token, tokengrpc::Value* response) override {

        string serialized_token;
        token->SerializeToString(&serialized_token);
    
        sgx_status_t ret = SGX_ERROR_UNEXPECTED;
        ret = ecall_distributed_PI(global_eid, serialized_token.c_str());
        if (ret != SGX_SUCCESS)
            abort();
        
        response->set_value("PARTIAL_INTERPOLATION_SUCCESS");

        return Status::OK;
    }

    Status Get_tokens(ServerContext* context, const tokengrpc::Node_id* source, tokengrpc::List_tokens* list_tokens){
        //printf("Helllllopooooo\n");
        char serialized_token[MAX_SIZE_SERIALIZED_TOKEN];
        memset(serialized_token, 'A', MAX_SIZE_SERIALIZED_TOKEN-1);
        
        sgx_status_t ret = SGX_ERROR_UNEXPECTED;
        
        int source_ = source->id();
        ret = ecall_get_tokens(global_eid, &source_, serialized_token);
        if (ret != SGX_SUCCESS)
            abort();

        //ocall_print_token(serialized_token);

        token::Token token;
        token.ParseFromString(serialized_token);

        *(list_tokens->add_tokens()) = token;


        return Status::OK;    
    }

}; 

class TokenClient{
    public:
        TokenClient(std::shared_ptr<Channel> channel): stub_(tokengrpc::Token::NewStub(channel)) {}

        string Partial_Polynomial_interpolation(token::Token token) {

            tokengrpc::Value reply;
            ClientContext context;

            CompletionQueue cq;
            Status status;


            std::unique_ptr<ClientAsyncResponseReader<tokengrpc::Value> > rpc(
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
        tokengrpc::Node_id request;
        tokengrpc::List_tokens list_tokens;

        int source_id = node_id_global;

        request.set_id(source_id);

        ClientContext context;
        Status status = stub_->Get_tokens(&context, request, &list_tokens);

        if (status.ok()){
            token::Token token = list_tokens.tokens(0);
            string serialized_token;
            token.SerializeToString(&serialized_token);

            return serialized_token;
        }else{
            return "null";
        }
    }

    private:
        unique_ptr<tokengrpc::Token::Stub> stub_;
    
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

void ocall_get_size_keys(int* size_keys){
    *size_keys = lost_keys_with_potential_last_share_owner_and_t_shares_owners.size();
}

void ocall_get_lost_key(char* lost_key){
    string lost_key_ = *it_lost_keys;
    it_lost_keys++;
    strncpy(lost_key, lost_key_.c_str(), strlen(lost_key));
}

void ocall_flush_lost_keys_list(){
    lost_keys_with_potential_last_share_owner_and_t_shares_owners.clear();
}



void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
    fflush(stdout);
}

void ocall_print_token(const char *serialized_token){
    token::Token token;

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
    TokenClient* token_client;
    token::Token token; 
    //printf("Enter Ocall send to next_node_id: %d\n", *next_node_id);

    token.ParseFromString(serialized_token);
    token_client = new TokenClient(grpc::CreateChannel(id_to_address_map[*next_node_id]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
    token_client->Partial_Polynomial_interpolation(token);
    delete token_client;
}

void ocall_get_tokens(int* node_id, char* serialized_token){
    TokenClient* token_client;
    
    token_client = new TokenClient(grpc::CreateChannel(id_to_address_map[*node_id]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
    string serialized_token_ = token_client->Get_tokens();
    
    //ocall_print_token(serialized_token_.c_str());
    
    delete token_client;

    //printf("and thennn ?\n");

    strncpy(serialized_token, serialized_token_.c_str(), strlen(serialized_token));

}

void ocall_delete_last_share(int* node_id, const char* key){
    KVSClient* kvs;
    printf("Share to delete: %s from %d\n",key, *node_id);

    kvs = new KVSClient(grpc::CreateChannel(id_to_address_map[*node_id]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
    kvs->Delete(key);
    delete kvs;
}


/*void test_share_lost_keys(){
    KVSClient* kvs;

    vector<int> S_up_ids;
    keyvaluestore::New_id_with_S_up_ids request;
    for(const auto& pair : id_to_address_map){
        if(pair.first != node_id_global){
            if(isPortOpen(pair.second, default_sms_port)) {
                S_up_ids.push_back(pair.first);
                request.add_s_up_ids(pair.first);
            }
        }
    }
    int N_active_size = S_up_ids.size();
    CompletionQueue cq;
    unique_ptr<keyvaluestore::KVS::Stub> stub;
    vector<ClientContext> contexts(N_active_size);
    std::unique_ptr<ClientReader<keyvaluestore::Lost_keys>> reader;
    keyvaluestore::New_id_with_S_up_ids request; 
    request.set_new_id(node_id_global);
    vector<keyvaluestore::Lost_keys> responses(N_active_size);
    vector<Status> statuses(N_active_size);
    int i=0;
    for (int s_up_id : S_up_ids){
        stub = keyvaluestore::KVS::NewStub(grpc::CreateChannel(id_to_address_map[s_up_id]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
        reader = stub->AsyncShare_lost_keys(&contexts[i], request, &cq);
        reader->Finish(&responses[i], &statuses[i], (void*)(i+1));
        i++:
    }
    while(true){

    }

}*/


void test_share_lost_keys(){
    KVSClient* kvs;

    vector<int> S_up_ids;

    for(const auto& pair : id_to_address_map){
        if(pair.first != node_id_global){
            if(isPortOpen(pair.second, default_sms_port)) {
                S_up_ids.push_back(pair.first);
                //std::cout << pair.first << std::endl;
            }
        }
    }

    for(int s_up_id : S_up_ids){
        kvs = new KVSClient(grpc::CreateChannel(id_to_address_map[s_up_id]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
        kvs->Share_lost_keys(node_id_global, S_up_ids); //******************************2
        delete kvs;
    }
}

void recover_lost_shares_wrapper(){
    recover_lost_shares();
}


ABSL_FLAG(uint16_t, t, 3, "number of necessary shares");
ABSL_FLAG(uint16_t, n, 5, "number of all shares");
ABSL_FLAG(uint16_t, node_id, 1, "node id");
ABSL_FLAG(uint16_t, recovery, 0, "recovery of shares");


int SGX_CDECL main(int argc, char *argv[]){ // ./app --node_id 3 --t 3 --n 5 --recovery 1

    id_to_address_map = parse_json("network.json");

    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }

    absl::ParseCommandLine(argc, argv);


    t_global = absl::GetFlag(FLAGS_t);
    n_global = absl::GetFlag(FLAGS_n);
    node_id_global = absl::GetFlag(FLAGS_node_id);
    int recovery = absl::GetFlag(FLAGS_recovery);

    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = ecall_send_params_to_enclave(global_eid, &node_id_global, &t_global, &n_global);
    if (ret != SGX_SUCCESS)
        abort();

    if(recovery){
        auto start_time = std::chrono::high_resolution_clock::now();
        test_share_lost_keys();//****************1
        cout << "Starting shares recovery... " << endl;
        recover_lost_shares_wrapper();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        cout << "Share recovery latency: " << duration.count() << " s" << endl;
    }


    KVSServiceImpl server;
    server.Run(default_sms_port);


    sgx_destroy_enclave(global_eid);

    printf("Server closed \n");

    return 0;
}



