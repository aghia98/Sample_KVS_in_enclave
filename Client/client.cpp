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
#include <vector>
#include <memory>
#include <string>
#include <unistd.h>
#include <getopt.h>  
#include <arpa/inet.h> //check if the grpc server port is open

#include "../gRPC_module/grpc_client.h"
#include "../SS_no_gmp_module/ss-client.h"
#include "../HRW_hashing_module/hrw.h"

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

   int Share_lost_keys(int id){
    New_id_with_S_up_ids request;
    request.set_new_id(id); // Replace with the desired ID value
    request.add_s_up_ids(33);
    request.add_s_up_ids(34);


    // Create a Lost_keys response
    Lost_keys response;

    ClientContext context;
    Status status = stub_->Share_lost_keys(&context, request, &response);

    if (status.ok()) {
        std::cout << "Lost Keys: ";
        for (const auto& key : response.keys()) {
            std::cout << key.key() << " ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "RPC failed";
    }

    return 0;

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


int number_of_open_ports_among_n(int starting_port, int n){
  int cpt=0;
  for(int i=0; i<n; i++){
    if(isPortOpen("127.0.0.1", starting_port+i)) cpt++;
  }
  return cpt;
}

void transmit_shares(string k, char** shares, vector<int> x_shares, string ip_address, int offset){ //the share (x_share, y_share) is sent to node port offset+x_share
  string v;
  string fixed = ip_address+":";
  string reply;
  KVSClient* kvs;
  int i=0;

  for(int x_share : x_shares){
    kvs = new KVSClient(grpc::CreateChannel(fixed+to_string(x_share+offset) , grpc::InsecureChannelCredentials()));
    v = shares[i];
    reply = kvs->Put(k,v);
    cout << "Client received: " << reply << endl;
    i++;

    delete kvs;
  }
}

/*void transmit_shares(string k, char** shares, int n, string ip_address, int port){
  string v;
  string fixed = ip_address+":";
  string reply;
  KVSClient* kvs;
  //int unavailable_nodes=0;

  for(int i=0; i<n; i++){ //transmitting shares
    if(isPortOpen("127.0.0.1", port+i)){
      kvs = new KVSClient(grpc::CreateChannel(fixed+to_string(port+i) , grpc::InsecureChannelCredentials()));
      v = shares[i];
      reply = kvs->Put(k,v);
      cout << "Client received: " << reply << endl;
    
      delete kvs;
    } 
  }
}*/

string* get_shares(string k, int n, int t, string ip_address, int port){
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
}

vector<int> get_ids_of_N_active(vector<int>& list_of_N_ports, int offset){
  vector<int> result;
  for(int port: list_of_N_ports){
    if (isPortOpen("127.0.0.1", port)) result.push_back(port-offset);
  }

  return result;
}

template <typename T>
void display_vector(const vector<T>& vec) {
    for (auto& element : vec) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

void test_share_lost_keys(){
  KVSClient* kvs;
  kvs = new KVSClient(grpc::CreateChannel("localhost:50001" , grpc::InsecureChannelCredentials()));
  kvs->Share_lost_keys(5);
  delete kvs;
}

/*int main(){
  test_share_lost_keys();
}*/

int main(int argc, char** argv) { // ./client -t x -n y --address localhost < secrets.txt

	seed_random();

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

  vector<int> list_of_N_ports = {50001,50002,50003,50004,50005}; int offset=50000;
  vector<int> ids_of_N_active;
  vector<string> strings_with_id_of_N_active;
  vector<pair<string, uint32_t>> ordered_strings_with_id_to_hash;

  int debug=0;
  int deg;
  string k;
  string v;
  char secret[200];
  string fixed = ip_address+":";
  string reply;
  

  cout << "Generating shares using a (" << t << "," << n << ") scheme with ";
  cout << "dynamic ";
  cout << "security level." << endl;

  //read secrets from file
  cin.sync_with_stdio(false);
  if (cin.rdbuf()->in_avail() != 0) {
      string line;
      int secret_num = 1;
      while (cin.getline(secret, sizeof(secret))) { //read secrets one by one
          
          ids_of_N_active = get_ids_of_N_active(list_of_N_ports, offset);

          if(ids_of_N_active.size() >= n){ // enough active SMS nodes
            k = "Secret_"+to_string(secret_num);
            strings_with_id_of_N_active = convert_ids_to_strings_with_id(ids_of_N_active, "server");
            //display_vector(strings_with_id_of_N_active);
            
            ordered_strings_with_id_to_hash = order_HRW(strings_with_id_of_N_active,k); //Order according to HRW

            vector<int> shares_x;

            for(int i=0; i<n;i++){ //extract top-n nodes'id
              auto pair = ordered_strings_with_id_to_hash[i];
              string server_with_id = pair.first;
              cout << "ID: " << server_with_id << ", Hash: " << pair.second << endl;
              shares_x.push_back(extractNumber(server_with_id));

            }
            
            //display_vector(shares_x);

            char ** shares = generate_share_strings(secret, n, t, shares_x);
          
            for(int i=0; i<n; i++) cout << shares[i] << endl; 

            //transmit_shares(k, shares, n, ip_address, port);

            transmit_shares(k, shares, shares_x, ip_address, offset);
            
            free_string_shares(shares, n);
            secret_num++; 

          }else{
            cout << "less than n=" << n << " SMS nodes are available. Please retry later." << endl;
          }
      }

      //***********************************************************************************************
      //string* got_shares;
      //if (number_of_open_ports_among_n(port, n) >= t){
        //got_shares = get_shares("Secret_1", n, t, ip_address, port);
        //for(int i=0; i<t; i++){
          //cout << got_shares[i] << endl;
        //}
        //delete[] got_shares;
      //} else{
        //cout << "less than t=" << t << " SMS nodes are available. Please retry later." << endl;
      //}
      
  } else {
      cerr << "No input provided through redirection." << endl;
      return 1;
    }

  return 0;
} 

