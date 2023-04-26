LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

INCLUDES += -I/home/aghia/usr/local/include 

all:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc client.cpp $(INCLUDES)
	g++ KVS_access.pb.o KVS_access.grpc.pb.o client.o $(LDFLAGS) -o client
	rm KVS_access.pb.o KVS_access.grpc.pb.o client.o

clean:
	rm client