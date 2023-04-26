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

class KVSClient {
 public:
  KVSClient(std::shared_ptr<Channel> channel)
      : stub_(KVS::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string Get(const int32_t k) {
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
      return std::to_string(reply.value());
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string Put(const int32_t k, const int32_t v) {
    // Follows the same pattern as SayHello.
    KV_pair request;
    request.set_key(k);
    request.set_value(v);
    Value reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Put(&context, request, &reply);
    if (status.ok()) {
      return std::to_string(reply.value());
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<KVS::Stub> stub_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  KVSClient kvs(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  
  int32_t k(1);
  int32_t v(10);

  std::string reply = kvs.Put(k,v);
  std::cout << "Client received: " << reply << std::endl;

  reply = kvs.Get(k);
  std::cout << "Client received: " << reply << std::endl;

  return 0;
}
