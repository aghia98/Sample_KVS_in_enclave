INCLUDES += -I/home/aghia/usr/local/include 

LDFLAGS += `pkg-config --libs --static protobuf grpc++ absl_flags absl_flags_parse`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

all:
	g++ -c KVS_access.pb.cc KVS_access.grpc.pb.cc $(INCLUDES)
	ar rcs libgrpckvs.a *.o
	mv libgrpckvs.a ..
	rm *.o

	