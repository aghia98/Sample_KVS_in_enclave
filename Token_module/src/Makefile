INCLUDES += -I/home/aghia/usr/local/include 

LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

all:
	g++ -c token.pb.cc grpc_token.pb.cc grpc_token.grpc.pb.cc $(INCLUDES)
	ar rcs libgrpctoken.a *.o
	mv libgrpctoken.a ..
	rm *.o

	