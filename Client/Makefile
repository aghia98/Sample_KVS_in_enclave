LDFLAGS_GRPC += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
		   
SOURCES_GRPC += ../gRPC_module/src/

LDFLAGS_SS += -L$(shell pwd)/../SS_no_gmp_module -lshamir_split

INCLUDES += -I/home/aghia/usr/local/include 

all:
	g++ -c $(SOURCES_GRPC)KVS_access.pb.cc $(SOURCES_GRPC)KVS_access.grpc.pb.cc client.cpp $(INCLUDES)
	g++ KVS_access.pb.o KVS_access.grpc.pb.o client.o $(LDFLAGS_GRPC) $(LDFLAGS_SS) -o client
	rm KVS_access.pb.o KVS_access.grpc.pb.o client.o

clean:
	rm client