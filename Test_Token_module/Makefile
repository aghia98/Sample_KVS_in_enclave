all:
	protoc --cpp_out=. token.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` grpc_token.proto
	protoc --cpp_out=. grpc_token.proto
	

