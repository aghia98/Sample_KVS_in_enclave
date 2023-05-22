LDFLAGS_GRPC += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
LDFLAGS_SS += -L../SS_no_gmp_module -lshamir_split	

LIBRARY_CLIENT_GRPC = ../gRPC_module/

SOURCES_GRPC += ../gRPC_module/src/
INCLUDES += -I/home/aghia/usr/local/include 

all:
	g++ -c client.cpp $(INCLUDES)
	g++ client.o -L$(LIBRARY_CLIENT_GRPC) -lgrpckvs $(LDFLAGS_SS) $(LDFLAGS_GRPC) -o client
	rm client.o

clean:
	rm client