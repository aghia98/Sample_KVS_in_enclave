#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <thread>
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

class KVSServiceImpl {
    public:
        KVSServiceImpl(){}
        
        template <typename T>
        static void createNew(KVSServiceImpl& parent, KVS::AsyncService& service, ServerCompletionQueue& cq) {
            new T(parent, service, cq);
        }

        class RequestBase{
            public:
                RequestBase(KVSServiceImpl& parent, KVS::AsyncService& service, ServerCompletionQueue& cq)
                    : parent_{parent}, service_{service}, cq_{cq} {}

                virtual ~RequestBase() = default;

                // The state-machine
                virtual void proceed(bool ok) = 0;

                void done() {
                    delete this;
                }

            protected:
                static size_t getNewReqestId() noexcept {
                    static size_t id = 0;
                    return ++id;
                }

                // The state required for all requests
                KVSServiceImpl& parent_;
                KVS::AsyncService& service_;
                ServerCompletionQueue& cq_;
                ServerContext ctx_;
                const size_t rpc_id_ = getNewReqestId();
            };

        class PutRequest : public RequestBase {
            public:

                PutRequest(KVSServiceImpl& parent, KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBase(parent, service, cq) {


                    service_.RequestPut(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {
                    if (state_ == CREATE) {
                        createNew<PutRequest>(parent_, service_, cq_);

                        std::thread([this]() {
                            myMap[request_.key()] = request_.value();
                            reply_.set_value("PUT_SUCCESS");

        
                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);
                        }).detach();
 

                    } else if (state_ == FINISH) {
                        done();  
                    } 
                    
                }

            private:
                KV_pair request_;
                Value reply_;
                ServerAsyncResponseWriter<Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
            };


        class GetRequest : public RequestBase{
            public:

                GetRequest(KVSServiceImpl& parent, KVS::AsyncService& service, ServerCompletionQueue& cq): RequestBase(parent, service, cq) {


                    service_.RequestGet(&ctx_, &request_, &responder_, &cq_, &cq_, this);
                }

                void proceed(bool ok) override {

                    if (state_ == CREATE) {
                        createNew<GetRequest>(parent_, service_, cq_);

                        std::thread([this]() {
                            auto iterator = myMap.find(request_.key());
                            if(iterator != myMap.end()){
                                reply_.set_value(iterator->second);
                            }else{
                                reply_.set_value("NOT_FOUND");
                            }

                            state_ = FINISH;
                            responder_.Finish(reply_, Status::OK, this);
                        }).detach();

                        
                    } else if (state_ == FINISH) {
                        done();  
                    } 
                }

            private:
                Key request_;
                Value reply_;
                ServerAsyncResponseWriter<Value> responder_{&ctx_};
                enum CallStatus { CREATE, FINISH };
                CallStatus state_ = CREATE;
        };


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

        void HandleRpcs() {
            createNew<PutRequest>(*this, service_, *cq_);
            createNew<GetRequest>(*this, service_, *cq_);
            

            // The inner event-loop
            while(true) {
                bool ok = true;
                void *tag = {};

                const auto status = cq_->Next(&tag, &ok);

                switch(status) {
                    case grpc::CompletionQueue::NextStatus::GOT_EVENT:
                        {
                            auto request = static_cast<RequestBase *>(tag);
                            request->proceed(ok);
                        }
                        break;

                    case grpc::CompletionQueue::NextStatus::SHUTDOWN:
                        return;
                } // switch
            } // loop
        }

    private:
        KVS::AsyncService service_;
        std::unique_ptr<grpc::ServerCompletionQueue> cq_;
        std::unique_ptr<grpc::Server> server_;
};


int main(int argc, char** argv) { // ./server --port 50001
    absl::ParseCommandLine(argc, argv);
    KVSServiceImpl server;
    server.Run(absl::GetFlag(FLAGS_port));

    return 0;
}
