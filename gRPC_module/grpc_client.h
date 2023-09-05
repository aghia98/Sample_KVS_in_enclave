#include "grpc_common.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::CompletionQueue;
using grpc::ClientAsyncResponseReader;

using keyvaluestore::KVS;
using keyvaluestore::Key;
using keyvaluestore::Value;
using keyvaluestore::KV_pair;
using keyvaluestore::New_id_with_S_up_ids;
using keyvaluestore::Lost_keys;
using keyvaluestore::List_tokens;
using keyvaluestore::Node_id;
using token::Token;

