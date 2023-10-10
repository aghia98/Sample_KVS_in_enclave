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



map<int, std::string> id_to_address_map;
int default_sms_port = 50000;


ABSL_FLAG(std::string, target, "localhost:50001", "Server address");



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

/*int number_of_open_ports_among_n(int starting_port, int n){
  int cpt=0;
  for(int i=0; i<n; i++){
    if(isPortOpen("127.0.0.1", starting_port+i)) cpt++;
  }
  return cpt;
}*/

void transmit_shares(string k, char** shares, vector<int> x_shares){ //the share (x_share, y_share) is sent to node port offset+x_share
  string v;
  string reply;
  KVSClient* kvs;
  int i=0;

  for(int x_share : x_shares){
    kvs = new KVSClient(grpc::CreateChannel(id_to_address_map[x_share]+":"+to_string(default_sms_port) , grpc::InsecureChannelCredentials()));
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


vector<int> get_ids_of_N_active(map<int,string>& id_to_address_map){
  vector<int> result;

  for (const auto& entry : id_to_address_map) {
    if (isPortOpen(entry.second, 50000)) result.push_back(entry.first);
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



int main(int argc, char** argv) { // ./client -t x -n y < secrets.txt

	seed_random();

  id_to_address_map = parse_json("network.json");
  

  char** shares;
  int t;
  int n;
  
  struct option long_options[] = {
      //{"address", required_argument, nullptr, 'a'},
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
          default:
              cerr << "Usage: " << argv[0] << " -t <t> -n <n>" << endl;
              return 1;
      }
  }

  //vector<int> list_of_N_ports = {50001,50002,50003,50004,50005,50006}; 
  vector<int> ids_of_N_active;
  vector<string> strings_with_id_of_N_active;
  vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;

  int debug=0;
  int deg;
  string k;
  string v;
  char secret[200];
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
          
          ids_of_N_active = get_ids_of_N_active(id_to_address_map);

          if(ids_of_N_active.size() >= n){ // enough active SMS nodes
            k = "Secret_"+to_string(secret_num);
            strings_with_id_of_N_active = convert_ids_to_strings_with_id(ids_of_N_active, "server");
            //display_vector(strings_with_id_of_N_active);
            
            ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,k); //Order according to HRW

            vector<int> shares_x;

            for(int i=0; i<n;i++){ //extract top-n nodes' id
              auto pair = ordered_strings_with_id_to_hash[i];
              string server_with_id = pair.first;
              //cout << "ID: " << server_with_id << ", Hash: " << pair.second << endl;
              shares_x.push_back(extractNumber(server_with_id));

            }
            
            //display_vector(shares_x);

            char ** shares = generate_share_strings(secret, n, t, shares_x);
          
            //for(int i=0; i<n; i++) cout << shares[i] << endl; 
            
            cout << " ( " << k << " , " << secret << " )\n" << endl;

            transmit_shares(k, shares, shares_x);

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

