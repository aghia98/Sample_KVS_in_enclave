#NOTICE: linking with -lpthread has been removed by me (Aghiles)

INCLUDE1 = $(HOME)/erpc/erpc_c/setup/
INCLUDE2 = $(HOME)/erpc/erpc_c/infra/
INCLUDE3 = $(HOME)/erpc/erpc_c/port/
INCLUDE4 = $(HOME)/erpc/erpc_c/config/
INCLUDE5 = $(HOME)/erpc/erpc_c/transports/
LIBRARY_erpckvs_client = ../eRPC_module/


all:
	g++ -c client.cpp -I${INCLUDE1} -I${INCLUDE2} -I${INCLUDE3} -I${INCLUDE4} -I${INCLUDE5}
	g++ client.o -L${LIBRARY_erpckvs_client} -lerpckvs_client -o client
	rm client.o

clean:
	rm client
	
