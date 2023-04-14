#include "erpc_access_KVS_server.h" 
#include "erpc_server_setup.h" 
#include "erpc_transport_setup.h" 
#include "erpc_mbf_setup.h" 

#include <iostream> 
#include <map>
using namespace std;
// Declare myMap as a global variable
map<int, int> myMap;


/* implementation of function call */
void erpcPut(int k, int v){
   
    myMap[k]=v;

}

void erpcGet(int k, int* v){
    *v =  myMap.find(k)->second;
}


int main()
{
    erpc_transport_t transport = erpc_transport_tcp_init("127.0.0.1",5407, true); 
    
    erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

    
    auto server = erpc_server_init(transport, message_buffer_factory);

    erpc_service_t service = create_AccessKVS_service();

    erpc_add_service_to_server(server, service);

    std::cout << erpc_server_run(server) << "\n"; 
    
   
    erpc_transport_tcp_close();

    std::cout << "Server closed" << std::endl;

    return 0;
}