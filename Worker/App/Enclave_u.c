#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_rebuild_secret_t {
	char* ms_combined_shares;
	char* ms_val;
	size_t ms_val_len;
} ms_ecall_rebuild_secret_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_Enclave = {
	1,
	{
		(void*)Enclave_ocall_print_string,
	}
};
sgx_status_t ecall_rebuild_secret(sgx_enclave_id_t eid, char combined_shares[1000], char* val)
{
	sgx_status_t status;
	ms_ecall_rebuild_secret_t ms;
	ms.ms_combined_shares = (char*)combined_shares;
	ms.ms_val = val;
	ms.ms_val_len = val ? strlen(val) + 1 : 0;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

