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
#include <unistd.h>

#include "../gRPC_module/grpc_common.h"
#include "../gRPC_module/grpc_client.h"
#include "../SS_no_gmp_module/ss-client.h"

using namespace std;


ABSL_FLAG(std::string, target, "localhost:50001", "Server address");


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

 private:
  unique_ptr<KVS::Stub> stub_;
};

void transmit_shares(string k, char** shares, int available_nodes, string ip_address, int port){
  string v;
  string target_str;
  string reply;
  KVSClient* kvs;

  for(int i=0; i<available_nodes; i++){ //transmitting shares
    target_str = ip_address+to_string(port+i); 
    kvs = new KVSClient(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    v = shares[i];
    reply = kvs->Put(k,v);
    cout << "Client received: " << reply << endl;
    
    delete kvs;
  }
}

int main(int argc, char** argv) { // ./client -t x -n y --target=localhost:60002

	seed_random();

  char** shares;
  int t;
  int n;
  int debug=0;
  int deg;
  int available_nodes = 2;
  int port = 50001;
  string k;
  string v;
  char secret[200];
  string target_str;
  string reply;
  string ip_address = "localhost:";
  KVSClient* kvs;

  int option;
      while ((option = getopt(argc, argv, "t:n:")) != -1) {
          switch (option) {
              case 't':
                  t = atoi(optarg);
                  break;
              case 'n':
                  n = atoi(optarg);
                  break;
              default:
                  cerr << "Usage: " << argv[0] << " -t <t> -n <n>" << endl;
                  return 1;
          }
      }

  cout << "Generating shares using a (" << t << "," << n << ") scheme with ";
  cout << "dynamic ";
  cout << "security level." << endl;

  //read secrets from file
  cin.sync_with_stdio(false);
  if (cin.rdbuf()->in_avail() != 0) {
      string line;
      int secret_num = 1;
      while (cin.getline(secret, sizeof(secret))) { //read secrets one by one
          char ** shares = generate_share_strings(secret, n, t);
          
          for(int i=0; i<n; i++) //display shares
            cout << shares[i] << endl; 

          k = "Secret "+to_string(secret_num);
          transmit_shares(k, shares, available_nodes, ip_address, port);

          secret_num++;
      }

      for(int i=0; i<available_nodes; i++){ //getting shares of secret 1
        target_str = ip_address+to_string(port+i); 
        kvs = new KVSClient(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
        reply = kvs->Get("Secret 1");
        cout << "Client received from node " << i+1 << ": " << reply << endl;
        
        delete kvs;
      }

      for(int i=0; i<available_nodes; i++){ //getting shares of secret 2
        target_str = ip_address+to_string(port+i); 
        kvs = new KVSClient(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
        reply = kvs->Get("Secret 2");
        cout << "Client received from node " << i+1 << ": " << reply << endl;
        
        delete kvs;
      }
  } else {
      cerr << "No input provided through redirection." << endl;
      return 1;
    }

  return 0;
}
