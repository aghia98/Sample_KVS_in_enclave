#include "grpc_common.h"

#include "absl/strings/str_format.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

//using tokengrpc::Token;
//using tokengrpc::Value;
//using tokengrpc::Node_id;
//using tokengrpc::List_tokens;
//using token::Token;
