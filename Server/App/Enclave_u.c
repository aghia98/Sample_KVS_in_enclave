#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_put_t {
	char* ms_key;
	char* ms_value;
} ms_ecall_put_t;

typedef struct ms_ecall_get_t {
	char* ms_key;
	char** ms_value;
} ms_ecall_get_t;

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
sgx_status_t ecall_put(sgx_enclave_id_t eid, char key[200], char value[200])
{
	sgx_status_t status;
	ms_ecall_put_t ms;
	ms.ms_key = (char*)key;
	ms.ms_value = (char*)value;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t ecall_get(sgx_enclave_id_t eid, char key[200], char** value)
{
	sgx_status_t status;
	ms_ecall_get_t ms;
	ms.ms_key = (char*)key;
	ms.ms_value = value;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

