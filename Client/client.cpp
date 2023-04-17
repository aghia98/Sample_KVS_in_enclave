#include <iostream> 
#include "../eRPC_module/erpc_common.h"
#include "../eRPC_module/erpc_client.h" 


int main()
{

    int v;
    /* Init eRPC client environment */
    /* UART transport layer initialization */
    erpc_transport_t transport = erpc_transport_tcp_init("127.0.0.1",5401, false);

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

    erpcGet(1, &v);
    erpcGet(2, &v);
    erpcGet(3, &v);
    erpcGet(4, &v);
    erpcGet(5, &v);
    erpcGet(6, &v);

    std::cout << "Value is " << v << '\n' ;


    
    erpc_transport_tcp_close();

    

    return 0;
}