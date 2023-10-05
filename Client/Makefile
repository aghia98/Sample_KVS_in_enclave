LIBRARY_GRPC_PATH = `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse` -lsystemd
LIBRARY_GRPCKVS_PATH = -L../gRPC_module
LIBRARY_SS_PATH = -L../SS_no_gmp_module
LIBRARY_HRW_PATH = -L../HRW_hashing_module

LDFLAGS_GRPCKVS = $(LIBRARY_GRPCKVS_PATH) -lgrpckvs
LDFLAGS_SS = $(LIBRARY_SS_PATH) -lshamir_split
LDFLAGS_HRW = $(LIBRARY_HRW_PATH) -lhrw
LDFLAGS_GRPC = $(LIBRARY_GRPC_PATH)\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl	

#SOURCES_GRPC += ../gRPC_module/src/
INCLUDES += -I$(HOME)/usr/local/include

all:
	g++ -c client.cpp $(INCLUDES)
	g++ *.o $(LDFLAGS_GRPCKVS) $(LDFLAGS_SS) $(LDFLAGS_HRW) $(LDFLAGS_GRPC) -o client
	rm *.o

clean:
	rm client