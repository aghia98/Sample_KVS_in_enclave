INCLUDES += -I/home/aghia/usr/local/include 

LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

all: server client

server:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc $(INCLUDES)
	ar rcs libgrpckvs_server.a *.o
	mv libgrpckvs_server.a ..
	rm *.o

client:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc $(INCLUDES)
	ar rcs libgrpckvs_client.a *.o
	mv libgrpckvs_client.a ..
	rm *.o
	