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

#include "../gRPC_module/grpc_common.h"
#include "../gRPC_module/grpc_client.h"
#include "../SS_module/ss-client.h"

using namespace std;


ABSL_FLAG(std::string, target, "localhost:50001", "Server address");


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

  std::string Put(const string k, const string v) {
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

 private:
  std::unique_ptr<KVS::Stub> stub_;
};

int main(int argc, char** argv) { // ./client --target=localhost:60002

  char** shares;
  int t=3;
  int n=5;
  int debug=0;
  int deg;

  char input[MAXLINELEN];

  printf("Generating shares using a (%d,%d) scheme with ", t, n);
  printf("dynamic");
  printf(" security level.\n");
  
  deg = MAXDEGREE;
  fprintf(stderr, "Enter the secret, ");
  fprintf(stderr, "at most %d ASCII characters: ", deg / 8);

  fgets(input, sizeof(input), stdin);

  shamir_split(t,n,debug, &shares, input);

  for(int i=0; i<5; i++)
    printf("%s\n", shares[i]); 
  //********************************************************************************************************
   absl::ParseCommandLine(argc, argv);
  
  std::string target_str = absl::GetFlag(FLAGS_target);
 
  KVSClient kvs(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  
  string k("aghiles");
  string v("this is randommmmmmmmm");

  string reply = kvs.Put(k,v);
  std::cout << "Client received: " << reply << std::endl;

  reply = kvs.Get(k);
  std::cout << "Client received: " << reply << std::endl;

  return 0;
}
