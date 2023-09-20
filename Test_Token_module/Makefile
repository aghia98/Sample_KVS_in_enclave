LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse` -pthread -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl

		   

INCLUDES += -I/home/aghia/usr/local/include 


all:
	protoc --cpp_out=. token.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` grpc_token.proto
	protoc --cpp_out=. grpc_token.proto
	

