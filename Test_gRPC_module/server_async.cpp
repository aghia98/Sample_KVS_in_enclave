#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include <map>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "KVS_access.grpc.pb.h"
#endif

// Declare myMap as a global variable
std::map<std::string, std::string> myMap;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

using keyvaluestore::KVS;
using keyvaluestore::Key;
using keyvaluestore::Value;
using keyvaluestore::KV_pair;

ABSL_FLAG(uint16_t, port, 50001, "Server port for the service");

class KVSServiceImpl final {
public:
    ~KVSServiceImpl() {
      server_->Shutdown();
      // Always shutdown the completion queue after the server.
      cq_->Shutdown();
  }
    void Run(uint16_t port) {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

        grpc::ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

private:
  
    class CallData {
    public:
        CallData(KVS::AsyncService* service, ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&context_), status_(CREATE) {
            Proceed();
            
        }

        void Proceed() {
            if (status_ == CREATE) {
                status_ = PROCESS;
                service_->RequestPut(&context_, &request_, &responder_, cq_, cq_, this);

            } else if (status_ == PROCESS) {
                new CallData(service_, cq_);

                myMap[request_.key()] = request_.value();
                reply_.set_value("PUT_SUCCESS");

                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

    private:
        KVS::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerContext context_;
        KV_pair request_;
        Value reply_;
        ServerAsyncResponseWriter<Value> responder_;
        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

    void HandleRpcs() {
        new CallData(&service_, cq_.get());
        void* tag;
        bool ok;
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<CallData*>(tag)->Proceed();
        }
    }

    std::unique_ptr<ServerCompletionQueue> cq_;
    KVS::AsyncService service_;
    std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) { // ./server --port 50001
    absl::ParseCommandLine(argc, argv);
    KVSServiceImpl server;
    server.Run(absl::GetFlag(FLAGS_port));

    return 0;
}
