LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

		   

INCLUDES += -I/home/aghia/usr/local/include 

all:
	protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` KVS_access.proto 
	protoc --cpp_out=. KVS_access.proto 

	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc server.cpp $(INCLUDES)
	g++ KVS_access.pb.o KVS_access.grpc.pb.o server.o $(LDFLAGS) -o server
	
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc client.cpp $(INCLUDES)
	g++ KVS_access.pb.o KVS_access.grpc.pb.o client.o $(LDFLAGS) -o client

clean:
	rm KVS_access.pb.o KVS_access.grpc.pb.o server.o server client.o client