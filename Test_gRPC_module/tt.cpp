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