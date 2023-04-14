#NOTICE: linking with -lpthread has been removed by me (Aghiles)

INCLUDE1 = $(HOME)/erpc/erpc_c/setup/
INCLUDE2 = $(HOME)/erpc/erpc_c/infra/
INCLUDE3 = $(HOME)/erpc/erpc_c/port/
INCLUDE4 = $(HOME)/erpc/erpc_c/config/
INCLUDE5 = $(HOME)/erpc/erpc_c/transports/
LIBRARY1 = $(HOME)/erpc/Release/Linux/erpc/lib/

all:
	g++ -c client.cpp erpc_access_KVS_client.cpp erpc_setup_tcp.cpp -I${INCLUDE1} -I${INCLUDE2} -I${INCLUDE3} -I${INCLUDE4} -I${INCLUDE5}
	g++ client.o erpc_access_KVS_client.o erpc_setup_tcp.o -L${LIBRARY1} -lerpc -o client

	g++ -c server.cpp erpc_access_KVS_server.cpp erpc_setup_tcp.cpp -I${INCLUDE1} -I${INCLUDE2} -I${INCLUDE3} -I${INCLUDE4} -I${INCLUDE5}
	g++ server.o erpc_access_KVS_server.o erpc_setup_tcp.o -L${LIBRARY1} -lerpc -o server

clean:
	rm client server client.o server.o erpc_access_KVS_client.o erpc_access_KVS_server.o erpc_setup_tcp.o
	
