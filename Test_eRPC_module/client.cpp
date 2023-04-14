#include <iostream> 
#include "erpc_access_KVS.h" 
#include "erpc_client_setup.h" //erpc_client_init()
#include "erpc_transport_setup.h" //erpc_transport_tcp_init()
#include "erpc_mbf_setup.h" //erpc_mbf_dynamic_init()


int main()
{

    int v;
    /* Init eRPC client environment */
    /* UART transport layer initialization */
    erpc_transport_t transport = erpc_transport_tcp_init("127.0.0.1",5407, false);

    /* MessageBufferFactory initialization */
    erpc_mbf_t message_buffer_factory = erpc_mbf_dynamic_init();

    /* eRPC client side initialization */
    erpc_client_init(transport, message_buffer_factory);

    /* other code like init matrix1 and matrix2 values */

    
    /* call eRPC functions */
    erpcPut(1, 10);
    erpcPut(2, 20);
    erpcPut(3, 30);
    erpcPut(4, 40);
    erpcPut(5, 50);
    erpcPut(6, 60);
    erpcGet(6, &v);

    std::cout << "Value is " << v << '\n' ;


    
    erpc_transport_tcp_close();

    

    return 0;
}