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
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <unistd.h>
#include <getopt.h>  
#include <arpa/inet.h> //check if the grpc server port is open

#include "../gRPC_module/grpc_client.h"
#include "../SS_no_gmp_module/ss-client.h"
#include "../HRW_hashing_module/hrw.h"

#include "../json/json.hpp"
using json = nlohmann::json;

using namespace std;



map<int, std::string> id_to_port_map;


ABSL_FLAG(std::string, target, "localhost:50001", "Server address");



class KVSClient {
 public:
  KVSClient(std::shared_ptr<Channel> channel): stub_(keyvaluestore::KVS::NewStub(channel)) {}

  string Get(const string k) {
    keyvaluestore::Key key;
    key.set_key(k);

    keyvaluestore::Value reply;

    grpc::ClientContext context;

    grpc::Status status = stub_->Get(&context, key, &reply);

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

 private:
  unique_ptr<keyvaluestore::KVS::Stub> stub_;
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

int number_of_open_ports_among_n(int starting_port, int n){
  int cpt=0;
  for(int i=0; i<n; i++){
    if(isPortOpen("127.0.0.1", starting_port+i)) cpt++;
  }
  return cpt;
}

void transmit_shares(string k, char** shares, vector<int> x_shares, string ip_address){ //the share (x_share, y_share) is sent to node port offset+x_share
  string v;
  string fixed = ip_address+":";
  string reply;
  KVSClient* kvs;
  int i=0;
  
  /*grpc::CompletionQueue cq;
  unique_ptr<keyvaluestore::KVS::Stub> stub;
  vector<ClientContext> contexts(x_shares.size());
  std::unique_ptr<grpc::ClientAsyncResponseReader<keyvaluestore::Value>> rpc;
  keyvaluestore::KV_pair request;
  vector<keyvaluestore::Value> responses(x_shares.size());
  vector<Status> statuses(x_shares.size());
  for (int i = 0; i < x_shares.size(); i++){
    request.set_key(k);
    request.set_value(shares[i]);
    stub = keyvaluestore::KVS::NewStub(grpc::CreateChannel(fixed+id_to_port_map[x_shares[i]] , grpc::InsecureChannelCredentials()));
    rpc = stub->AsyncPut(&contexts[i], request, &cq);
    rpc->Finish(&responses[i], &statuses[i], (void*)(i+1));
  }
  int num_responses_received = 0;
  while (num_responses_received < x_shares.size()){
    void* got_tag;
    bool ok = false;
    cq.Next(&got_tag, &ok);
    if (ok){
      int response_index = reinterpret_cast<intptr_t>(got_tag) - 1;
      if (statuses[response_index].ok()) {
        cout << "Share transmitted to node id = " << x_shares[response_index] << ": " << endl;
        cout << shares[response_index] << endl;
        printf("\n");
      } else {

      }
      num_responses_received++;
    }
  }*/


  for(int x_share : x_shares){
    kvs = new KVSClient(grpc::CreateChannel(fixed+id_to_port_map[x_share] , grpc::InsecureChannelCredentials()));
    v = shares[i];
    reply = kvs->Put(k,v);
    cout << "Share transmitted to node id = " << x_share << ": " << endl;
    cout << v << endl;
    cout << "Result: " << reply << endl;
    i++;
    printf("\n");

    delete kvs;
  }
}


/*string* get_shares(string k, int n, int t, string ip_address, int port){
  string fixed = ip_address+":";
  string reply;
  KVSClient* kvs;
  int got_shares=0;
  int node_id=0;
  string* shares = new string[t];

  while(got_shares<t){
    if(isPortOpen("127.0.0.1", port+node_id)){
      kvs = new KVSClient(grpc::CreateChannel(fixed+to_string(port+node_id), grpc::InsecureChannelCredentials()));
      shares[got_shares] = kvs->Get(k);
      //cout << "Client received from node at port " << port+node_id << ": " << reply << endl;

      delete kvs;
      got_shares++;
    }
    node_id++;
  }

  return shares;
}*/

vector<int> get_ids_of_N_active(){
  vector<int> result;

  for (const auto& entry : id_to_port_map) {
    if (isPortOpen("127.0.0.1", stoi(entry.second))) result.push_back(entry.first);
  }

  return result;
}

/*template <typename T>
void display_vector(const vector<T>& vec) {
    for (auto& element : vec) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
} */



int main(int argc, char** argv) { // ./client -t x -n y --address localhost < secrets.txt

	seed_random();


  id_to_port_map = parse_json("network.json");
  

  char** shares;
  int t;
  int n;
  string ip_address;
  
  struct option long_options[] = {
      {"address", required_argument, nullptr, 'a'},
      {nullptr, 0, nullptr, 0}
  };

  int option;
  while ((option = getopt_long(argc, argv, "t:n:", long_options, nullptr)) != -1) {
      switch (option) {
          case 't':
              t = atoi(optarg);
              break;
          case 'n':
              n = atoi(optarg);
              break;
          case 'a':
              ip_address = optarg;
              break;
          default:
              cerr << "Usage: " << argv[0] << " -t <t> -n <n> --address <address>" << endl;
              return 1;
      }
  }

  //vector<int> list_of_N_ports = {50001,50002,50003,50004,50005,50006}; 
  vector<int> ids_of_N_active;
  vector<string> strings_with_id_of_N_active;
  vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;

  string k;
  string v;
  char secret[200];
  string fixed = ip_address+":";
  string reply;
  

  cout << "Generating shares using a (" << t << "," << n << ") scheme with ";
  cout << "dynamic ";
  cout << "security level." << endl;

  cout << "\nSecrets' split :\n" << endl;

  //read secrets from file
  cin.sync_with_stdio(false);
  if (cin.rdbuf()->in_avail() != 0) {
      string line;
      int secret_num = 1;
      while (cin.getline(secret, sizeof(secret))) { //read secrets one by one
          
          ids_of_N_active = get_ids_of_N_active();

          if(ids_of_N_active.size() >= n){ // enough active SMS nodes
            k = "Secret_"+to_string(secret_num);
            strings_with_id_of_N_active = convert_ids_to_strings_with_id(ids_of_N_active, "server");
            //display_vector(strings_with_id_of_N_active);
            
            ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,k); //Order according to HRW

            vector<int> shares_x;

            for(int i=0; i<n;i++){ //extract top-n nodes'id
              auto pair = ordered_strings_with_id_to_hash[i];
              string server_with_id = pair.first;
              //cout << "ID: " << server_with_id << ", Hash: " << pair.second << endl;
              shares_x.push_back(extractNumber(server_with_id));

            }
            
            //display_vector(shares_x);

            char ** shares = generate_share_strings(secret, n, t, shares_x);
          
            //for(int i=0; i<n; i++) cout << shares[i] << endl; 
            
            cout << " ( " << k << " , " << secret << " )\n" << endl;

            transmit_shares(k, shares, shares_x, ip_address);

            cout << "\n-------------------------------------------\n" << endl;
            
            free_string_shares(shares, n);
            secret_num++; 

          }else{
            cout << "less than n=" << n << " SMS nodes are available. Please retry later." << endl;
          }
      }

  } else {
      cerr << "No input provided through redirection." << endl;
      return 1;
    }

  return 0;
} 

