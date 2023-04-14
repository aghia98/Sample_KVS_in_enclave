#This script builds the liberpckvs (erpc KVS library) static library

INCLUDE1 = $(HOME)/erpc/erpc_c/setup/
INCLUDE2 = $(HOME)/erpc/erpc_c/infra/
INCLUDE3 = $(HOME)/erpc/erpc_c/port/
INCLUDE4 = $(HOME)/erpc/erpc_c/config/
INCLUDE5 = $(HOME)/erpc/erpc_c/transports/
LIBRARY1 = $(HOME)/erpc/Release/Linux/erpc/lib/

#Build Static library
server:
	g++ -c erpc_access_KVS_server.cpp erpc_setup_tcp.cpp -I${INCLUDE1} -I${INCLUDE2} -I${INCLUDE3} -I${INCLUDE4} -I${INCLUDE5}
	ar x ${LIBRARY1}liberpc.a --output .
	ar rcs liberpckvs_server.a *.o 
	mv liberpckvs_server.a ..
	rm *.o

client:
	g++ -c erpc_access_KVS_client.cpp erpc_setup_tcp.cpp -I${INCLUDE1} -I${INCLUDE2} -I${INCLUDE3} -I${INCLUDE4} -I${INCLUDE5}
	ar x ${LIBRARY1}liberpc.a --output .
	ar rcs liberpckvs_client.a *.o 
	mv liberpckvs_client.a ..
	rm *.o

clean:
	rm liberpckvs_client.a liberpckvs_server.a
	
