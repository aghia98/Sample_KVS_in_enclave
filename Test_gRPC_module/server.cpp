/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <stdio.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "KVS_access.grpc.pb.h"
#endif

#include <map>
using namespace std;
// Declare myMap as a global variable
map<string,string> myMap;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using keyvaluestore::KVS;
using keyvaluestore::Key;
using keyvaluestore::Value;
using keyvaluestore::KV_pair;
using keyvaluestore::New_id_with_S_up_ids;
using keyvaluestore::Lost_keys;

ABSL_FLAG(uint16_t, port, 50001, "Server port for the service");

// Logic and data behind the server's behavior.
class KVSServiceImpl final : public KVS::Service {
  //Get(::grpc::ClientContext* context, const ::keyvaluestore::Key& request, ::keyvaluestore::Value* response)
  Status Get(ServerContext* context, const Key* key, Value* value) override {
    auto iterator = myMap.find(key->key());
    if(iterator != myMap.end()){
      value->set_value(iterator->second);
    }else{
      value->set_value("NOT_FOUND");
    }
    return Status::OK;
  }

  Status Put(ServerContext* context, const KV_pair* request, Value* response) override{
    myMap[request->key()]=request->value();
    response->set_value("PUT_SUCCESS");
    return Status::OK;
  }

  Status Delete(ServerContext* context, const Key* key, Value* response) override{
    myMap.erase(key->key());
    response->set_value("DELETE_SUCCESS");
    return Status::OK;
  }

  Status Share_lost_keys(ServerContext* context, const New_id_with_S_up_ids* request, Lost_keys* response) override {
        int id = request->new_id();
        //std::vector<int32_t> s_up_ids;
        
        cout << id << endl;
        for (const auto& s_up_id : request->s_up_ids()) {
            //s_up_ids.push_back(id);
            cout << s_up_id << endl;
        }
        
        Key* key = response->add_keys();
        key->set_key("example");

        return Status::OK;
  }


};

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
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) { // ./server --port 50001
  absl::ParseCommandLine(argc, argv);
  RunServer(absl::GetFlag(FLAGS_port));
  return 0;
}
