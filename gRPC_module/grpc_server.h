#include "grpc_common.h"

#include "absl/strings/str_format.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using keyvaluestore::KVS;
using keyvaluestore::Key;
using keyvaluestore::Value;
using keyvaluestore::KV_pair;
using keyvaluestore::New_id_with_S_up_ids;
using keyvaluestore::Lost_keys;
using keyvaluestore::List_tokens;
using keyvaluestore::Node_id;
using token::Token;

