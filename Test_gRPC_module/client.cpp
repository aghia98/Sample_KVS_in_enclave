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

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "KVS_access.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50001", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using keyvaluestore::KVS;
using keyvaluestore::Key;
using keyvaluestore::Value;
using keyvaluestore::KV_pair;

using namespace std;

class KVSClient {
 public:
  KVSClient(std::shared_ptr<Channel> channel)
      : stub_(KVS::NewStub(channel)) {}

  string Get(const string k) {
    // Data we are sending to the server.
    Key key;
    key.set_key(k);

    // Container for the data we expect from the server.
    Value reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
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
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

    string Delete(const string k) {
    // Follows the same pattern as SayHello.
    Key key;
    key.set_key(k);
    Value reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Delete(&context, key, &reply);
    if (status.ok()) {
      return reply.value();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<KVS::Stub> stub_;
};


int main(int argc, char** argv) { // ./client --target localhost:50001
  absl::ParseCommandLine(argc, argv);
  
  std::string target_str = absl::GetFlag(FLAGS_target);
 
  KVSClient kvs(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  
  string k("aghiles");
  string v("0758908571");

  string reply = kvs.Put(k,v);
  std::cout << "Put....Client received: " << reply << std::endl;

  reply = kvs.Get(k);
  std::cout << "Get....Client received: " << reply << std::endl;

  reply = kvs.Delete(k);
  std::cout << "Delete ....Client received: " << reply << std::endl;

  reply = kvs.Get(k);
  std::cout << "Get ....Client received: " << reply << std::endl;

  

  return 0;
}
