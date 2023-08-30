LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse` -pthread -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl

		   

INCLUDES += -I/home/aghia/usr/local/include 

all: protoc client server
	 
protoc:
	protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` KVS_access.proto 
	protoc --cpp_out=. KVS_access.proto

client:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc token.pb.cc client.cpp $(INCLUDES)
	g++ *.o $(LDFLAGS) -o client
	rm *.o

server:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc token.pb.cc server.cpp $(INCLUDES)
	g++ *.o $(LDFLAGS) -o server
	rm *.o


clean:
	rm client server