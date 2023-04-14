#This script builds the liberpckvs (erpc KVS library) static library

INCLUDE1 = $(HOME)/erpc/erpc_c/setup/
INCLUDE2 = $(HOME)/erpc/erpc_c/infra/
INCLUDE3 = $(HOME)/erpc/erpc_c/port/
INCLUDE4 = $(HOME)/erpc/erpc_c/config/
INCLUDE5 = $(HOME)/erpc/erpc_c/transports/
LIBRARY1 = $(HOME)/erpc/Release/Linux/erpc/lib/

#Generates Static then dynamic library
all:
	cd src && make server && make client
	
clean:
	rm liberpckvs_server.a liberpckvs_client.a
	
